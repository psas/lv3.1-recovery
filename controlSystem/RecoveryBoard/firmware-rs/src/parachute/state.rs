use embassy_time::Instant;
use ufmt::uwrite;

#[derive(Debug)]
pub enum ChuteStateField {
    Ready(bool),
    ShorePowerOn(bool),
    SenderLastSeen(u64),
}

#[derive(Default)]
pub struct ChuteState {
    pub ready: bool,
    pub shore_power_on: bool,
    pub sender_last_seen: u64,
}

impl ChuteState {
    pub fn iter(&self) -> ChuteStateIter<'_> {
        ChuteStateIter { state_fields: self, index: 0 }
    }
}

pub struct ChuteStateIter<'a> {
    state_fields: &'a ChuteState,
    index: usize,
}

impl<'a> Iterator for ChuteStateIter<'a> {
    type Item = ChuteStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(ChuteStateField::Ready(self.state_fields.ready)),
            1 => Some(ChuteStateField::ShorePowerOn(self.state_fields.shore_power_on)),
            2 => Some(ChuteStateField::SenderLastSeen(self.state_fields.sender_last_seen)),
            _ => None,
        };

        if result.is_some() {
            self.index += 1;
        }

        result
    }
}

impl core::fmt::Display for ChuteStateField {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        let time_now = Instant::now().as_millis();
        match *self {
            Self::Ready(val) => {
                core::write!(f, "Ready: {}", if val { "YES" } else { "NO" })
            }
            Self::ShorePowerOn(val) => {
                core::write!(f, "Shore Power: {}", if val { "ON" } else { "OFF" })
            }
            Self::SenderLastSeen(val) => core::write!(f, "Sender last seen: {}ms", time_now - val),
        }
    }
}

impl ufmt::uDisplay for ChuteStateField {
    fn fmt<W>(&self, f: &mut ufmt::Formatter<'_, W>) -> Result<(), W::Error>
    where
        W: ufmt::uWrite + ?Sized,
    {
        let time_now = Instant::now().as_millis();
        match *self {
            Self::Ready(val) => uwrite!(f, "Ready: {}", if val { "YES\r\n" } else { "NO\r\n" }),
            Self::ShorePowerOn(val) => {
                uwrite!(f, "Shore Power: {}", if val { "ON\r\n" } else { "OFF\r\n" })
            }
            Self::SenderLastSeen(val) => uwrite!(f, "Sender last seen: {}ms\r\n", time_now - val),
        }
    }
}
