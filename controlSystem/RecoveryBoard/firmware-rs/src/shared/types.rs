use embassy_stm32::{
    adc::Adc,
    can::Can,
    dac::Dac,
    gpio::Input,
    mode::Async,
    peripherals::{ADC1, DAC1, TIM15},
    timer::simple_pwm::SimplePwm,
};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    mutex::Mutex,
    signal::Signal,
};

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type UmbOnType = Mutex<ThreadModeRawMutex, Option<Input<'static>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;
pub type AdcType = Mutex<CriticalSectionRawMutex, Option<Adc<'static, ADC1>>>;
pub type DacType = Mutex<ThreadModeRawMutex, Option<Dac<'static, DAC1, Async>>>;
pub type BattOkSignalType = Signal<CriticalSectionRawMutex, u8>;
pub type ShortPowOnSignalType = Signal<CriticalSectionRawMutex, bool>;
