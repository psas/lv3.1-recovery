use defmt::*;
use embassy_stm32::{
    dac::{Dac, Value},
    gpio::{Input, Level, Output, Pull, Speed},
    mode::Async,
    peripherals::{DAC1, PB4, PB5, PB6, PB7},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex, watch::Watch};
use embassy_time::{with_timeout, Duration};

use crate::ring::RingPosition;

// TODO: set up adc channel to read motor_isense while driving the motor in order to plot data.

pub struct Motor {
    pub deploy1: Output<'static>,
    pub deploy2: Output<'static>,
    pub ps: Output<'static>,
    pub motor_fail: Input<'static>,
    pub dac: Dac<'static, DAC1, Async>,
    ring_pos_watch: &'static Watch<ThreadModeRawMutex, RingPosition, 5>,
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
        ring_pos_watch: &'static Watch<ThreadModeRawMutex, RingPosition, 5>,
    ) -> Self {
        let deploy1 = Output::new(pb4, Level::Low, Speed::Medium);
        let deploy2 = Output::new(pb5, Level::Low, Speed::Medium);
        let ps = Output::new(pb6, Level::High, Speed::Medium);
        let motor_fail = Input::new(pb7, Pull::Up);

        Self { deploy1, deploy2, ps, motor_fail, dac, ring_pos_watch }
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
        debug!("step = {}", scale);
        let val = Value::Bit12Right(scale);
        self.dac.ch1().set(val);
    }

    async fn read_ring_pos_until_condition(&mut self, position: RingPosition) {
        let mut receiver = self.ring_pos_watch.receiver().expect("Could not get receiver");
        loop {
            let ring_position = receiver.changed().await;
            if ring_position == position {
                break;
            }
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
    }
}

pub type MotorType = Mutex<ThreadModeRawMutex, Option<Motor>>;
