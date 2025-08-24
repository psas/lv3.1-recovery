use embassy_stm32::{can::Can, peripherals::TIM15, timer::simple_pwm::SimplePwm};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex};

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;
