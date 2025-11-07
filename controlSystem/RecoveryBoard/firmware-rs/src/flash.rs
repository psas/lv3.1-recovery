use embassy_sync::mutex::Mutex;

use crate::types::FlashType;

// Motor actuations count sector
pub const MOTOR_ACT_SECTOR_OFFSET: u32 = 0x3F000; // Secctor 64 Addr: 0x0803_F000
pub const MOTOR_ACT_SECTOR_SIZE: u32 = 4096; // Sector size is 4Kbytes

pub static FLASH_MTX: FlashType = Mutex::new(None);
