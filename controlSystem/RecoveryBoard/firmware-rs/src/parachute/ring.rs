use defmt::{error, info, Format};
use embassy_stm32::{
    adc::SampleTime,
    peripherals::{PA0, PA1, PB1},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex, signal::Signal, watch::Watch};
use embassy_time::Timer;
use ufmt::{uDisplay, uwrite};

use crate::{
    adc::ADC_MTX,
    flash::{FLASH_MTX, SENSOR_LIMIT_SECTOR_OFFSET, SENSOR_LIMIT_SECTOR_SIZE},
};

pub type RingType = Mutex<ThreadModeRawMutex, Option<Ring>>;

pub static RING_MTX: RingType = Mutex::new(None);

pub static RING_POSITION_WATCH: Watch<ThreadModeRawMutex, RingPosition, 5> = Watch::new();
pub static SENSOR_READ_WATCH: Watch<ThreadModeRawMutex, SensorReadings, 5> = Watch::new();
pub static MOTOR_ISENSE_SIGNAL: Signal<ThreadModeRawMutex, u16> = Signal::new();

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

#[derive(Default, Format)]
pub struct SensorLimits {
    pub over: u16,
    pub under: u16,
    pub active: u16,
    pub unactive: u16,
}

impl uDisplay for SensorLimits {
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        uwrite!(
            f,
            "over: {}\r\nunder: {}\r\nactive: {}\r\nunactive: {}\r\n",
            self.over,
            self.under,
            self.active,
            self.unactive
        )
    }
}

impl SensorLimits {
    pub fn new(over: u16, under: u16, active: u16, unactive: u16) -> Self {
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

impl uDisplay for RingPosition {
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        match *self {
            RingPosition::Locked => uwrite!(f, "Locked\r\n"),
            RingPosition::Unlocked => uwrite!(f, "Unlocked\r\n"),
            RingPosition::Inbetween => uwrite!(f, "Inbetween\r\n"),
            RingPosition::Error => uwrite!(f, "Error\r\n"),
        }
    }
}

impl uDisplay for SensorState {
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        match *self {
            Self::Active => {
                uwrite!(f, "Active\r\n")
            }
            Self::Unactive => {
                uwrite!(f, "Unactive\r\n")
            }
            Self::Under => {
                uwrite!(f, "Under\r\n")
            }
            Self::Over => uwrite!(f, "Over\r\n"),
            Self::Inbetween => uwrite!(f, "Inbetween\r\n"),
        }
    }
}

impl uDisplay for SensorReadings {
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        uwrite!(
            f,
            "Sensor 1:\r\n- state: {}\r\n- value: {}\r\nSensor 2:\r\n- state: {}\r\n- value: {}\r\n",
            self.sensor1_state,
            self.sensor1,
            self.sensor2_state,
            self.sensor2
        )
    }
}

pub struct Ring {
    pa0: Peri<'static, PA0>,
    pa1: Peri<'static, PA1>,
    pb1: Peri<'static, PB1>,
    pub sensor1_limits: SensorLimits,
    pub sensor2_limits: SensorLimits,
}

impl Ring {
    pub async fn new(
        pa0: Peri<'static, PA0>,
        pa1: Peri<'static, PA1>,
        pb1: Peri<'static, PB1>,
    ) -> Self {
        let sensor1_limits: SensorLimits;
        let sensor2_limits: SensorLimits;

        let mut flash_unlocked = FLASH_MTX.lock().await;
        if let Some(flash) = flash_unlocked.as_mut() {
            let mut buf = [0u8; (SENSOR_LIMIT_SECTOR_SIZE / 8) as usize];

            if let Err(e) = flash.blocking_read(SENSOR_LIMIT_SECTOR_OFFSET, &mut buf) {
                error!("Error reading sensor limits from flash: {}", e);
            }

            fn two_u8_to_u16(b1: u8, b0: u8) -> u16 {
                ((b1 as u16) << 8) + b0 as u16
            }

            sensor1_limits = SensorLimits::new(
                two_u8_to_u16(buf[0], buf[1]),
                two_u8_to_u16(buf[2], buf[3]),
                two_u8_to_u16(buf[4], buf[5]),
                two_u8_to_u16(buf[6], buf[7]),
            );
            sensor2_limits = SensorLimits::new(
                two_u8_to_u16(buf[8], buf[9]),
                two_u8_to_u16(buf[10], buf[11]),
                two_u8_to_u16(buf[12], buf[13]),
                two_u8_to_u16(buf[14], buf[15]),
            );
            info!("sensor1: {}", sensor1_limits);
            info!("sensor2: {}", sensor2_limits)
        } else {
            error!("Error reading sensor limits from flash. Using default values");
            sensor1_limits = SensorLimits::new(3700, 600, 2100, 900);
            sensor2_limits = SensorLimits::new(3700, 600, 1300, 900);
        }

        Self { pa0, pa1, pb1, sensor1_limits, sensor2_limits }
    }

    pub async fn broadcast_ring_position(&mut self) {
        let ring_position_sender = RING_POSITION_WATCH.sender();
        let sensor_reading_sender = SENSOR_READ_WATCH.sender();

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
                sensor1_read = adc.read(&mut self.pa0, SampleTime::CYCLES239_5).await;
                sensor2_read = adc.read(&mut self.pa1, SampleTime::CYCLES239_5).await;
                motor_isense_read = adc.read(&mut self.pb1, SampleTime::CYCLES239_5).await;
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
        MOTOR_ISENSE_SIGNAL.signal(motor_isense_read);

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
        Timer::after_millis(15).await;
    }
}
