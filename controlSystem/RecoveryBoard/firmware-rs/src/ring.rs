use defmt::debug;
use defmt::error;
use embassy_stm32::peripherals::PA0;
use embassy_stm32::peripherals::PA1;
use embassy_stm32::Peri;
use embassy_sync::blocking_mutex::raw::ThreadModeRawMutex;
use embassy_sync::mutex::Mutex;
use embassy_sync::watch::Watch;
use embassy_time::Timer;

use crate::types::AdcType;

pub type RingType = Mutex<ThreadModeRawMutex, Option<Ring>>;

pub static RING_POSITION_WATCH: Watch<ThreadModeRawMutex, RingPosition, 5> = Watch::new();

#[derive(defmt::Format, PartialEq, Clone)]
pub enum RingPosition {
    Locked,
    Unlocked,
    Inbetween,
    Error,
}

#[derive(Default)]
struct SensorLimits {
    over: u16,
    under: u16,
    active: u16,
    unactive: u16,
}

impl SensorLimits {
    fn new(over: u16, under: u16, active: u16, unactive: u16) -> Self {
        Self { over, under, active, unactive }
    }
}

#[derive(PartialEq)]
enum SensorState {
    Active,
    Unactive,
    Under,
    Over,
    Inbetween,
}

pub struct Ring {
    pa0: Peri<'static, PA0>,
    pa1: Peri<'static, PA1>,
    sensor1_limits: SensorLimits,
    sensor2_limits: SensorLimits,
    adc_mtx: &'static AdcType,
}

impl Ring {
    pub fn new(
        pa0: Peri<'static, PA0>,
        pa1: Peri<'static, PA1>,
        adc_mtx: &'static AdcType,
    ) -> Self {
        let sensor1_limits = SensorLimits::new(3700, 600, 2100, 900);
        let sensor2_limits = SensorLimits::new(3700, 600, 1300, 900);

        Self {
            pa0,
            pa1,
            sensor1_limits,
            sensor2_limits,
            adc_mtx,
        }
    }

    pub async fn broadcast_ring_position(&mut self) {
        let sender = RING_POSITION_WATCH.sender();

        fn get_sensor_state(adc_val: u16, limit: &SensorLimits) -> SensorState {
            if adc_val >= limit.over {
                SensorState::Over
            } else if adc_val <= limit.under {
                SensorState::Under
            } else if adc_val >= limit.active && adc_val < limit.over {
                SensorState::Active
            } else if adc_val <= limit.unactive && adc_val > limit.under {
                SensorState::Unactive
            } else {
                SensorState::Inbetween
            }
        }

        fn get_ring_position(
            sensor1_state: SensorState,
            sensor2_state: SensorState,
        ) -> RingPosition {
            if sensor1_state == SensorState::Active && sensor2_state != SensorState::Active {
                return RingPosition::Unlocked;
            }
            if sensor1_state != SensorState::Unactive && sensor2_state == SensorState::Unactive {
                return RingPosition::Unlocked;
            }
            if sensor1_state != SensorState::Active && sensor2_state == SensorState::Active {
                return RingPosition::Locked;
            }
            if sensor1_state == SensorState::Unactive && sensor2_state != SensorState::Unactive {
                return RingPosition::Locked;
            }
            if sensor1_state == SensorState::Inbetween || sensor2_state == SensorState::Inbetween {
                RingPosition::Inbetween
            } else {
                RingPosition::Error
            }
        }

        let mut sensor1_read = 0u16;
        let mut sensor2_read = 0u16;

        {
            let mut adc_unlocked = self.adc_mtx.lock().await;
            if let Some(adc) = adc_unlocked.as_mut() {
                sensor1_read = adc.read(&mut self.pa0).await;
                sensor2_read = adc.read(&mut self.pa1).await;
            }
        }

        let sensor1_state = get_sensor_state(sensor1_read, &self.sensor1_limits);
        let sensor2_state = get_sensor_state(sensor2_read, &self.sensor2_limits);

        let ring_position = get_ring_position(sensor1_state, sensor2_state);

        match ring_position {
            RingPosition::Locked => debug!("1: {} 2: {} - Ring Locked", sensor1_read, sensor2_read),
            RingPosition::Unlocked => {
                debug!("1: {} 2: {} - Ring Unlocked", sensor1_read, sensor2_read)
            }
            RingPosition::Inbetween => {
                debug!("1: {} 2: {} - Ring Inbetween", sensor1_read, sensor2_read)
            }
            RingPosition::Error => error!("1: {} 2: {} - Ring Error", sensor1_read, sensor2_read),
        }

        sender.send(ring_position);
    }
}

#[embassy_executor::task]
pub async fn read_pos_sensor(ring: &'static RingType) {
    loop {
        {
            let mut ring_unlocked = ring.lock().await;
            if let Some(ring) = ring_unlocked.as_mut() {
                ring.broadcast_ring_position().await;
            }
        }
        Timer::after_millis(100).await;
    }
}
