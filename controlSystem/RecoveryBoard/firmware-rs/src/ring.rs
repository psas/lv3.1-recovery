use embassy_stm32::peripherals::PA0;
use embassy_stm32::peripherals::PA1;
use embassy_stm32::peripherals::PB1;
use embassy_stm32::Peri;
use embassy_sync::blocking_mutex::raw::ThreadModeRawMutex;
use embassy_sync::mutex::Mutex;
use embassy_sync::watch::Watch;
use embassy_time::Timer;

use crate::adc::ADC_MTX;

pub type RingType = Mutex<ThreadModeRawMutex, Option<Ring>>;

pub static RING_MTX: RingType = Mutex::new(None);

pub static RING_POSITION_WATCH: Watch<ThreadModeRawMutex, RingPosition, 5> = Watch::new();
pub static SENSOR_READ_WATCH: Watch<ThreadModeRawMutex, SensorReadings, 5> = Watch::new();
pub static MOTOR_ISENSE_WATCH: Watch<ThreadModeRawMutex, u16, 1> = Watch::new();

#[derive(defmt::Format, PartialEq, Clone)]
pub enum RingPosition {
    Locked,
    Unlocked,
    Inbetween,
    Error,
}

#[derive(Clone)]
pub struct SensorReadings {
    pub sensor1: u16,
    pub sensor1_state: SensorState,
    pub sensor2: u16,
    pub sensor2_state: SensorState,
}

impl SensorReadings {
    pub fn new(
        sensor1: u16,
        sensor1_state: SensorState,
        sensor2: u16,
        sensor2_state: SensorState,
    ) -> Self {
        Self { sensor1, sensor1_state, sensor2, sensor2_state }
    }
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

#[derive(PartialEq, Clone)]
pub enum SensorState {
    Active,
    Unactive,
    Under,
    Over,
    Inbetween,
}

impl core::fmt::Display for SensorState {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match *self {
            Self::Active => {
                core::write!(f, "Active")
            }
            Self::Unactive => {
                core::write!(f, "Unactive")
            }
            Self::Under => {
                core::write!(f, "Under")
            }
            Self::Over => core::write!(f, "Over"),
            Self::Inbetween => core::write!(f, "Inbetween"),
        }
    }
}

pub struct Ring {
    pa0: Peri<'static, PA0>,
    pa1: Peri<'static, PA1>,
    pb1: Peri<'static, PB1>,
    sensor1_limits: SensorLimits,
    sensor2_limits: SensorLimits,
}

impl Ring {
    pub fn new(pa0: Peri<'static, PA0>, pa1: Peri<'static, PA1>, pb1: Peri<'static, PB1>) -> Self {
        let sensor1_limits = SensorLimits::new(3700, 600, 2100, 900);
        let sensor2_limits = SensorLimits::new(3700, 600, 1300, 900);

        Self { pa0, pa1, pb1, sensor1_limits, sensor2_limits }
    }

    pub async fn broadcast_ring_position(&mut self) {
        let ring_position_sender = RING_POSITION_WATCH.sender();
        let sensor_reading_sender = SENSOR_READ_WATCH.sender();
        let motor_isense_sender = MOTOR_ISENSE_WATCH.sender();

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
        let mut motor_isense_read = 0u16;

        {
            let mut adc_unlocked = ADC_MTX.lock().await;
            if let Some(adc) = adc_unlocked.as_mut() {
                sensor1_read = adc.read(&mut self.pa0).await;
                sensor2_read = adc.read(&mut self.pa1).await;
                motor_isense_read = adc.read(&mut self.pb1).await;
            }
        }

        let sensor1_state = get_sensor_state(sensor1_read, &self.sensor1_limits);
        let sensor2_state = get_sensor_state(sensor2_read, &self.sensor2_limits);

        let readings = SensorReadings::new(
            sensor1_read,
            sensor1_state.clone(),
            sensor2_read,
            sensor2_state.clone(),
        );

        sensor_reading_sender.send(readings);
        motor_isense_sender.send(motor_isense_read);

        let ring_position = get_ring_position(sensor1_state, sensor2_state);

        ring_position_sender.send(ring_position);
    }
}

#[embassy_executor::task]
pub async fn read_pos_sensor() {
    loop {
        {
            let mut ring_unlocked = RING_MTX.lock().await;
            if let Some(ring) = ring_unlocked.as_mut() {
                ring.broadcast_ring_position().await;
            }
        }
        Timer::after_millis(50).await;
    }
}
