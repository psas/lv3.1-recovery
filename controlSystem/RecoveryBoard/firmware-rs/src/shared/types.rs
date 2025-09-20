use embassy_stm32::{
    can::{Can, Frame},
    gpio::Input,
    peripherals::TIM15,
    timer::simple_pwm::SimplePwm,
};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    mutex::Mutex,
    signal::Signal,
};

pub struct CanTxChannelMsg {
    pub blocking: bool,
    pub frame: Frame,
}

impl CanTxChannelMsg {
    pub fn new(blocking: bool, frame: Frame) -> Self {
        Self { blocking, frame }
    }
}

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type UmbOnType = Mutex<ThreadModeRawMutex, Option<Input<'static>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;

pub type BattOkSignalType = Signal<CriticalSectionRawMutex, u8>;
pub type ShortPowOnSignalType = Signal<CriticalSectionRawMutex, bool>;
