/* Sender board specific state code */

use embassy_time::Instant;
use ufmt::uwrite;

#[derive(Debug)]
pub enum SenderStateField {
    RocketReady(bool),
    ForceRocketReady(bool),
    DrogueReady(bool),
    MainReady(bool),
    ShorePowerOn(bool),
    DrogueLastSeen(u64),
    MainLastSeen(u64),
    IsoMainLastSeen(u64),
    IsoDrogueLastSeen(u64),
}

pub struct SenderState {
    pub rocket_ready: bool,
    pub force_rocket_ready: bool,
    pub drogue_ready: bool,
    pub main_ready: bool,
    pub shore_pow_on: bool,
    pub drogue_last_seen: u64,
    pub main_last_seen: u64,
    pub iso_main_last_seen: u64,
    pub iso_drogue_last_seen: u64,
}

impl Default for SenderState {
    fn default() -> Self {
        Self {
            rocket_ready: false,
            force_rocket_ready: false,
            drogue_ready: false,
            main_ready: false,
            shore_pow_on: true,
            drogue_last_seen: 0,
            main_last_seen: 0,
            iso_main_last_seen: 0,
            iso_drogue_last_seen: 0,
        }
    }
}

impl SenderState {
    pub fn iter(&self) -> SenderStateIter<'_> {
        SenderStateIter { state_fields: self, index: 0 }
    }
}

pub struct SenderStateIter<'a> {
    state_fields: &'a SenderState,
    index: usize,
}

impl<'a> Iterator for SenderStateIter<'a> {
    type Item = SenderStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(SenderStateField::RocketReady(self.state_fields.rocket_ready)),
            1 => Some(SenderStateField::ForceRocketReady(self.state_fields.force_rocket_ready)),
            2 => Some(SenderStateField::DrogueReady(self.state_fields.drogue_ready)),
            3 => Some(SenderStateField::MainReady(self.state_fields.main_ready)),
            4 => Some(SenderStateField::ShorePowerOn(self.state_fields.shore_pow_on)),
            5 => Some(SenderStateField::DrogueLastSeen(self.state_fields.drogue_last_seen)),
            6 => Some(SenderStateField::MainLastSeen(self.state_fields.main_last_seen)),
            7 => Some(SenderStateField::IsoMainLastSeen(self.state_fields.iso_main_last_seen)),
            8 => Some(SenderStateField::IsoDrogueLastSeen(self.state_fields.iso_drogue_last_seen)),
            _ => None,
        };

        if result.is_some() {
            self.index += 1;
        }

        result
    }
}

impl core::fmt::Display for SenderStateField {
    // impl display for the state fields so they can be written by defmt
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        let time_now_ms = Instant::now().as_millis();
        match *self {
            Self::RocketReady(val) => {
                core::write!(f, "Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            Self::ForceRocketReady(val) => {
                core::write!(f, "Force Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            Self::DrogueReady(val) => {
                core::write!(f, "Drogue Status: {}", if val { "OK" } else { "NOT READY" })
            }
            Self::MainReady(val) => {
                core::write!(f, "Main Status: {}", if val { "OK" } else { "NOT READY" })
            }
            Self::ShorePowerOn(val) => {
                core::write!(f, "Shore Power: {}", if val { "ON" } else { "OFF" })
            }
            Self::DrogueLastSeen(val) => {
                core::write!(f, "Drogue last seen: {}ms", time_now_ms - val)
            }
            Self::MainLastSeen(val) => core::write!(f, "Main last seen: {}ms", time_now_ms - val),
            Self::IsoDrogueLastSeen(val) => {
                core::write!(f, "Iso drogue last seen: {}ms", time_now_ms - val)
            }
            Self::IsoMainLastSeen(val) => {
                core::write!(f, "Iso main last seen: {}ms", time_now_ms - val)
            }
        }
    }
}

impl ufmt::uDisplay for SenderStateField {
    // impl udisplay for state fields so they can be written over uart
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        let time_now_ms = Instant::now().as_millis();
        match *self {
            Self::RocketReady(val) => {
                uwrite!(f, "Rocket Ready: {}", if val { "YES\r\n" } else { "NO\r\n" })
            }
            Self::ForceRocketReady(val) => {
                uwrite!(f, "Force Rocket Ready: {}", if val { "YES\r\n" } else { "NO\r\n" })
            }
            Self::DrogueReady(val) => {
                uwrite!(f, "Drogue Status: {}", if val { "OK\r\n" } else { "NOT READY\r\n" })
            }
            Self::MainReady(val) => {
                uwrite!(f, "Main Status: {}", if val { "OK\r\n" } else { "NOT READY\r\n" })
            }
            Self::ShorePowerOn(val) => {
                uwrite!(f, "Shore Power: {}", if val { "ON\r\n" } else { "OFF\r\n" })
            }
            Self::DrogueLastSeen(val) => {
                uwrite!(f, "Drogue last seen: {}ms\r\n", time_now_ms - val)
            }
            Self::MainLastSeen(val) => ufmt::uwrite!(f, "Main last seen: {}ms\r\n", time_now_ms - val),
            Self::IsoDrogueLastSeen(val) => {
                uwrite!(f, "Iso drogue last seen: {}ms\r\n", time_now_ms - val)
            }
            Self::IsoMainLastSeen(val) => {
                uwrite!(f, "Iso main last seen: {}ms\r\n", time_now_ms - val)
            }
        }
    }
}
