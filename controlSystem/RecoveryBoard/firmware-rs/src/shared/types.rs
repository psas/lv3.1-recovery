use embassy_stm32::{
    adc::Adc,
    can::Can,
    gpio::Input,
    peripherals::{ADC1, TIM15},
    timer::simple_pwm::SimplePwm,
};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    mutex::Mutex,
    signal::Signal,
};

#[derive(Default)]
pub struct SenderState {
    pub rocket_ready: bool,
    pub drogue_status: bool,
    pub main_status: bool,
    pub drogue_last_seen: u64,
    pub main_last_seen: u64,
    pub iso_main_last_seen: u64,
    pub iso_drogue_last_seen: u64,
}

#[derive(Debug)]
pub enum SenderStateField {
    RocketReady(bool),
    DrogueStatus(bool),
    MainStatus(bool),
    DrogueLastSeen(u64),
    MainLastSeen(u64),
    IsoMainLastSeen(u64),
    IsoDrogueLastSeen(u64),
}

pub struct SenderStateIter<'a> {
    state_fields: &'a SenderState,
    index: usize,
}

impl SenderState {
    pub fn iter(&self) -> SenderStateIter<'_> {
        SenderStateIter { state_fields: self, index: 0 }
    }
}

impl<'a> Iterator for SenderStateIter<'a> {
    type Item = SenderStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(SenderStateField::RocketReady(self.state_fields.rocket_ready)),
            1 => Some(SenderStateField::DrogueStatus(self.state_fields.drogue_status)),
            2 => Some(SenderStateField::MainStatus(self.state_fields.main_status)),
            3 => Some(SenderStateField::DrogueLastSeen(self.state_fields.drogue_last_seen)),
            4 => Some(SenderStateField::MainLastSeen(self.state_fields.main_last_seen)),
            5 => Some(SenderStateField::IsoMainLastSeen(self.state_fields.iso_main_last_seen)),
            6 => Some(SenderStateField::IsoDrogueLastSeen(self.state_fields.iso_drogue_last_seen)),
            _ => None,
        };

        if result.is_some() {
            self.index += 1;
        }

        result
    }
}

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type UmbOnType = Mutex<ThreadModeRawMutex, Option<Input<'static>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;
pub type AdcType = Mutex<CriticalSectionRawMutex, Option<Adc<'static, ADC1>>>;
pub type SenderStateType = Mutex<ThreadModeRawMutex, Option<SenderState>>;

pub type BattOkSignalType = Signal<CriticalSectionRawMutex, u8>;
pub type ShortPowOnSignalType = Signal<CriticalSectionRawMutex, bool>;
