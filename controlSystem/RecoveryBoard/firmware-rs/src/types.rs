use embassy_stm32::{
    adc::Adc,
    can::{BufferedCan, Can},
    dac::Dac,
    flash::{Blocking, Flash},
    gpio::Input,
    mode::Async,
    peripherals::{ADC1, DAC1, TIM15},
    timer::simple_pwm::SimplePwm,
};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    mutex::Mutex,
};

use crate::can::CAN_BUF_SIZE;

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type UmbOnType = Mutex<ThreadModeRawMutex, Option<Input<'static>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<BufferedCan<'static, CAN_BUF_SIZE, CAN_BUF_SIZE>>>;
pub type AdcType = Mutex<CriticalSectionRawMutex, Option<Adc<'static, ADC1>>>;
pub type DacType = Mutex<ThreadModeRawMutex, Option<Dac<'static, DAC1, Async>>>;
pub type FlashType = Mutex<ThreadModeRawMutex, Option<Flash<'static, Blocking>>>;
