use embassy_stm32::{
    adc::Adc,
    can::Can,
    peripherals::{ADC1, TIM15},
    timer::simple_pwm::SimplePwm,
};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    mutex::Mutex,
    signal::Signal,
};

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;
pub type AdcType = Mutex<ThreadModeRawMutex, Option<Adc<'static, ADC1>>>;

pub type BattOkSignalType = Signal<CriticalSectionRawMutex, bool>;
