/*
* Provides control and management of the parachute deployment motor system. It handles motor
* direction, current limiting, and position-based control with safety features.

*/

use defmt::*;
use embassy_stm32::{
    dac::{Dac, Value},
    gpio::{Input, Level, Output, Pull, Speed},
    mode::Async,
    peripherals::{PB4, PB5, PB6, PB7},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex, watch::Receiver};
use embassy_time::{with_timeout, Duration};

use crate::{
    flash::{FLASH_MTX, MOTOR_ACT_SECTOR_OFFSET, MOTOR_ACT_SECTOR_SIZE},
    parachute::ring::{RingPosition, MOTOR_ISENSE_SIGNAL, RING_POSITION_WATCH},
};

pub static MOTOR_MTX: MotorType = Mutex::new(None);

pub const MOTOR_DRIVE_DUR_MS: u64 = 1000; // Maximum current limit for motor operation
pub const MOTOR_DRIVE_CURR_MA: u16 = 1000; // Default timeout duration for motor operations

pub struct Motor {
    pub deploy1: Output<'static>, // Output pin for motor direction control (forward)
    pub deploy2: Output<'static>, // Output pin for motor direction control (reverse)
    pub ps: Output<'static>,      // Power save control pin
    pub motor_fail: Input<'static>, // Input pin for motor failure detection
    pub dac: Dac<'static, Async>, // DAC peripheral for current limiting control
    pub ring_pos_receiver: Receiver<'static, ThreadModeRawMutex, RingPosition, 5>,
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
        pb4: Peri<'static, PB4>, // deploy1
        pb5: Peri<'static, PB5>, // deploy2
        pb6: Peri<'static, PB6>, // power save
        pb7: Peri<'static, PB7>, // motor_fail
        dac: Dac<'static, Async>,
    ) -> Self {
        let deploy1 = Output::new(pb4, Level::Low, Speed::Medium);
        let deploy2 = Output::new(pb5, Level::Low, Speed::Medium);
        let ps = Output::new(pb6, Level::High, Speed::Medium);
        let motor_fail = Input::new(pb7, Pull::Up);
        let ring_pos_receiver =
            unwrap!(RING_POSITION_WATCH.receiver(), "Could not get ring pos receiver for motor");

        Self { deploy1, deploy2, ps, motor_fail, dac, ring_pos_receiver }
    }

    pub fn set_mode(&mut self, mode: MotorMode) {
        // Sets the motor operating mode by controlling the output pins.
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
        /* Use the dac to limit the current coming out of the motor driver.
         * Math is from calculating the dac value from circuit analysis going backwards from motor current.
         * First we do mA*3/10, which gives us MOTOR_VREF, then multiply by 2 to get MOTOR_ILIM.
         * Lastly the dac takes in a 11 bit value which represents a scalar of the max voltage
         * (3.3V). We use mV*3300/4096 to get the step size from the output voltage.
         * Combining all the numerators and denominators to avoid overflow we get 1024/1375
         */

        if ma >= 2000 {
            error!("Current too high!");
            return;
        };

        let scale = ((ma as u32) * 1024 / 1375) as u16;
        let val = Value::Bit12Right(scale);
        self.dac.ch1().set(val);
    }

    async fn read_ring_pos_until(&mut self, position: RingPosition) {
        // read the ring position until it is in the desired position then break
        const BUFSIZE: usize = 64; // INFO If running the motor for longer, increase this
        let mut buf = [0u16; BUFSIZE];
        let mut count = 0usize;
        loop {
            if count < BUFSIZE {
                buf[count] = MOTOR_ISENSE_SIGNAL.wait().await;
            }
            let ring_position = self.ring_pos_receiver.changed().await;
            count = count.wrapping_add(1);
            if ring_position == position {
                break;
            }
        }
        // INFO Only gets to this point if the ring reaches the desired position
        debug!("Motor_isense: {}", buf[..count]);
    }

    async fn increment_actuation_count(&self) {
        // increments the flash memory stored actuation count
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
        // clear actuation count stored in flash
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
        /* Drive the motor towards the specified position
         * Passing in force will drive until timeout, ignoring ring position
         *
         * Operation:
         * 1. Sets current limit via DAC
         * 2. Stops motor initially
         * 3. Drives in specified direction based on target position
         * 4. Monitors either position feedback or uses timeout
         * 5. Returns to power save mode when complete
         */
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
                    let limit = self.read_ring_pos_until(RingPosition::Locked);
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
                    let limit = self.read_ring_pos_until(RingPosition::Unlocked);
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
