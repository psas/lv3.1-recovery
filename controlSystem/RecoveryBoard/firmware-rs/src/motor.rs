use defmt::*;
use embassy_stm32::{
    dac::{Dac, Value},
    gpio::{Input, Level, Output, Pull, Speed},
    mode::Async,
    peripherals::{DAC1, PB4, PB5, PB6, PB7},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex};
use embassy_time::{with_timeout, Duration};

use crate::{
    flash::{FLASH_MTX, MOTOR_ACT_SECTOR_OFFSET, MOTOR_ACT_SECTOR_SIZE},
    ring::{RingPosition, MOTOR_ISENSE_WATCH, RING_POSITION_WATCH},
};

pub const MOTOR_DRIVE_DUR_MS: u64 = 1000;
pub const MOTOR_DRIVE_CURR_MA: u16 = 1000;

pub struct Motor {
    pub deploy1: Output<'static>,
    pub deploy2: Output<'static>,
    pub ps: Output<'static>,
    pub motor_fail: Input<'static>,
    pub dac: Dac<'static, DAC1, Async>,
}

pub enum MotorMode {
    PowerSave,
    Stop,
    Forward,
    Reverse,
    Brake,
}

impl Motor {
    pub fn new(
        pb4: Peri<'static, PB4>,
        pb5: Peri<'static, PB5>,
        pb6: Peri<'static, PB6>,
        pb7: Peri<'static, PB7>,
        dac: Dac<'static, DAC1, Async>,
    ) -> Self {
        let deploy1 = Output::new(pb4, Level::Low, Speed::Medium);
        let deploy2 = Output::new(pb5, Level::Low, Speed::Medium);
        let ps = Output::new(pb6, Level::High, Speed::Medium);
        let motor_fail = Input::new(pb7, Pull::Up);

        Self { deploy1, deploy2, ps, motor_fail, dac }
    }

    pub fn set_mode(&mut self, mode: MotorMode) {
        match mode {
            MotorMode::PowerSave => {
                self.ps.set_low();
                self.deploy1.set_low();
                self.deploy2.set_low();
            }
            MotorMode::Stop => {
                self.ps.set_high();
                self.deploy1.set_low();
                self.deploy2.set_low();
            }
            MotorMode::Forward => {
                self.ps.set_high();
                self.deploy1.set_high();
                self.deploy2.set_low();
            }
            MotorMode::Reverse => {
                self.ps.set_high();
                self.deploy1.set_low();
                self.deploy2.set_high();
            }
            MotorMode::Brake => {
                self.ps.set_high();
                self.deploy1.set_high();
                self.deploy2.set_high();
            }
        }
    }

    async fn limit_motor_current(&mut self, ma: u16) {
        if ma >= 2000 {
            error!("Current too high!");
            return;
        };
        // math is calculating the dac value from circuit analysis going backwards from motor current.
        // First we do mA*3/10, which gives us MOTOR_VREF, then multiply by 2 to get MOTOR_ILIM and
        // then lastly the dac takes in a 11 bit value which represents a scalar of the max voltage
        // (3.3V). Which we use mV*3300/4096 to get the step size from the output voltage.
        // Combining all the numerators and denominators to avoid overflow we get 1024/1375
        let scale = ((ma as u32) * 1024 / 1375) as u16;
        let val = Value::Bit12Right(scale);
        self.dac.ch1().set(val);
    }

    async fn read_ring_pos_until_condition(&mut self, position: RingPosition) {
        let mut ring_pos_receiver =
            RING_POSITION_WATCH.receiver().expect("Could not get ring_pos rcvr");
        let mut isense_receiver = MOTOR_ISENSE_WATCH.receiver().expect("Could not get isense rcvr");
        const BUFSIZE: usize = 64; // INFO If running the motor for longer, increase this
        let mut buf = [0u16; BUFSIZE];
        let mut count = 0usize;
        loop {
            if count < BUFSIZE {
                buf[count] = isense_receiver.changed().await;
            }
            let ring_position = ring_pos_receiver.changed().await;
            count = count.wrapping_add(1);
            if ring_position == position {
                break;
            }
        }
        // INFO Only gets to this point if the ring reaches the desired position
        debug!("Motor_isense: {}", buf[..count]);
    }

    async fn increment_actuation_count(&self) {
        let mut flash_unlocked = FLASH_MTX.lock().await;
        if let Some(flash) = flash_unlocked.as_mut() {
            let mut buf = [0u8; (MOTOR_ACT_SECTOR_SIZE / 8) as usize];

            if let Err(e) = flash.blocking_read(MOTOR_ACT_SECTOR_OFFSET, &mut buf) {
                error!("Error reading actuation count from memory: {}", e);
            }

            buf[0] = buf[0].wrapping_add(1);

            // Sector must be erased before writing or SEQ err will be thrown
            if let Err(e) = flash.blocking_erase(
                MOTOR_ACT_SECTOR_OFFSET,
                MOTOR_ACT_SECTOR_OFFSET + MOTOR_ACT_SECTOR_SIZE,
            ) {
                error!("Error erasing memory: {}", e);
            }

            if let Err(e) = flash.blocking_write(MOTOR_ACT_SECTOR_OFFSET, &buf) {
                error!("Error writing actuation count to memory: {}", e);
            }
        }
    }

    pub async fn erase_actuation_data(&self) {
        let mut flash_unlocked = FLASH_MTX.lock().await;
        if let Some(flash) = flash_unlocked.as_mut() {
            info!("Erasing motor actuation data");
            if let Err(e) = flash.blocking_erase(
                MOTOR_ACT_SECTOR_OFFSET,
                MOTOR_ACT_SECTOR_OFFSET + MOTOR_ACT_SECTOR_SIZE,
            ) {
                error!("Error erasing memory: {}", e);
            }
            info!("Motor actuation data erased");
        }
    }

    pub async fn drive(&mut self, mode: RingPosition, duration_ms: u64, force: bool, current: u16) {
        self.limit_motor_current(current).await;
        self.set_mode(MotorMode::Stop);
        match mode {
            RingPosition::Locked => {
                self.set_mode(MotorMode::Reverse);
                if force {
                    let limit = core::future::pending::<()>();
                    if let Err(e) = with_timeout(Duration::from_millis(duration_ms), limit).await {
                        error!("Motor limit timed out: {}", e);
                    }
                } else {
                    let limit = self.read_ring_pos_until_condition(RingPosition::Locked);
                    if let Err(e) = with_timeout(Duration::from_millis(duration_ms), limit).await {
                        error!("Motor limit timed out: {}", e);
                    }
                }
            }
            RingPosition::Unlocked => {
                self.set_mode(MotorMode::Forward);
                if force {
                    let limit = core::future::pending::<()>();
                    if let Err(e) = with_timeout(Duration::from_millis(duration_ms), limit).await {
                        error!("Motor limit timed out: {}", e);
                    }
                } else {
                    let limit = self.read_ring_pos_until_condition(RingPosition::Unlocked);
                    if let Err(e) = with_timeout(Duration::from_millis(duration_ms), limit).await {
                        error!("Motor limit timed out: {}", e);
                    }
                }
            }
            _ => error!("Unhandled drive mode. Choose Locked or Unlocked"),
        }

        self.set_mode(MotorMode::PowerSave);
        self.increment_actuation_count().await;
    }
}

pub type MotorType = Mutex<ThreadModeRawMutex, Option<Motor>>;
