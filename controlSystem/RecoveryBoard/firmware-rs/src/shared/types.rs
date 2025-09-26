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
    pub force_rocket_ready: bool,
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
    ForceRocketReady(bool),
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
            1 => Some(SenderStateField::ForceRocketReady(self.state_fields.force_rocket_ready)),
            2 => Some(SenderStateField::DrogueStatus(self.state_fields.drogue_status)),
            3 => Some(SenderStateField::MainStatus(self.state_fields.main_status)),
            4 => Some(SenderStateField::DrogueLastSeen(self.state_fields.drogue_last_seen)),
            5 => Some(SenderStateField::MainLastSeen(self.state_fields.main_last_seen)),
            6 => Some(SenderStateField::IsoMainLastSeen(self.state_fields.iso_main_last_seen)),
            7 => Some(SenderStateField::IsoDrogueLastSeen(self.state_fields.iso_drogue_last_seen)),
            _ => None,
        };

        if result.is_some() {
            self.index += 1;
        }

        result
    }
}

impl core::fmt::Display for SenderStateField {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            &Self::RocketReady(val) => {
                write!(f, "Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            &Self::ForceRocketReady(val) => {
                write!(f, "Force Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            &Self::DrogueStatus(val) => {
                write!(f, "Drogue Status: {}", if val { "OK" } else { "ERR" })
            }
            &Self::MainStatus(val) => write!(f, "Main Status: {}", if val { "OK" } else { "ERR" }),
            Self::DrogueLastSeen(val) => write!(f, "Drogue last seen: {}ms", val),
            Self::MainLastSeen(val) => write!(f, "Main last seen: {}ms", val),
            Self::IsoDrogueLastSeen(val) => write!(f, "Iso drogue last seen: {}ms", val),
            Self::IsoMainLastSeen(val) => write!(f, "Iso main last seen: {}ms", val),
        }
    }
}

pub type PwmType = Mutex<ThreadModeRawMutex, Option<SimplePwm<'static, TIM15>>>;
pub type UmbOnType = Mutex<ThreadModeRawMutex, Option<Input<'static>>>;
pub type CanType = Mutex<ThreadModeRawMutex, Option<Can<'static>>>;
pub type AdcType = Mutex<CriticalSectionRawMutex, Option<Adc<'static, ADC1>>>;
pub type SenderStateType = Mutex<ThreadModeRawMutex, Option<SenderState>>;

pub type BattOkSignalType = Signal<CriticalSectionRawMutex, u8>;
pub type ShortPowOnSignalType = Signal<CriticalSectionRawMutex, bool>;
