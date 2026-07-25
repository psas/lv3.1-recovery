/* Parachute board specific cli commands code */

use defmt::{error, info, Format};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, channel::Channel};
use embedded_cli::Command;

use crate::{
    buzzer::{BuzzerMode, BUZZER_MODE_MTX},
    flash::{FLASH_MTX, SENSOR_LIMIT_SECTOR_OFFSET, SENSOR_LIMIT_SECTOR_SIZE},
    parachute::{
        motor::{MOTOR_DRIVE_CURR_MA, MOTOR_DRIVE_DUR_MS, MOTOR_MTX},
        ring::{RingPosition, SensorLimits, RING_MTX},
    },
};

pub static ASYNC_CMD_CHANNEL: Channel<CriticalSectionRawMutex, AsyncCmd, 1> = Channel::new();

#[derive(Debug, Command, Format, Clone, Copy)]
pub enum ChuteCmd<'a> {
    /// Print internal state
    State,

    /// Print current battery voltage
    Batt,

    /// Toggle periodic beep
    Beep,

    /// Move the ring towards the lock position.
    L {
        /// Ignore sensor readings, continue until timeout.
        #[arg(long)]
        force: bool,

        /// Do a 100ms step instead of a full swing.
        #[arg(long)]
        pulse: bool,
    },

    /// Move the ring towards the unlocked position.
    U {
        /// Ignore sensor readings, continue until timeout.
        #[arg(long)]
        force: bool,

        /// Do a 100ms step instead of a full swing.
        #[arg(long)]
        pulse: bool,
    },

    /// Print the current sensor readings and ring state.
    Pos,

    /// Print the number of motor actuations stored in flash.
    Acts,

    /// Erase motor actuation count data from flash.
    Erase,

    /// Set or read the current sensor limits.
    ///
    /// Usage: limits over1,under1,active1,unactive1,over2,under2,active2,unactive2
    Limits {
        /// Print the current sensor limits.
        #[arg(long)]
        print: bool,

        /// Sensor limits as comma-separated values.
        #[arg(long)]
        set: Option<&'a str>,
    },

    /// Print info about the current firmware version
    Version,
}

#[allow(clippy::large_enum_variant)] // ~300 bytes is no big deal when it moves 1 time
#[derive(Debug)]
pub enum AsyncCmd {
    Beep,
    L { force: bool, pulse: bool },
    U { force: bool, pulse: bool },
    SetLimits { limits: heapless::Vec<heapless::String<32>, 10> },
    Erase,
}

#[embassy_executor::task]
pub async fn async_cmd_handler() {
    /* This task is more or less a hack to get the sync embedded-cli-rs crate to play nice with our
     * async runtime. It will await commands coming in on the channel and dispatch any async code
     * needed to handle said command.
     */
    loop {
        match ASYNC_CMD_CHANNEL.receive().await {
            AsyncCmd::Beep => {
                let mut buzz_mode_unlocked = BUZZER_MODE_MTX.lock().await;
                if let Some(mode) = buzz_mode_unlocked.as_mut() {
                    match mode {
                        BuzzerMode::Off => {
                            info!("Setting Buzzer mode to low");
                            *mode = BuzzerMode::Low;
                        }
                        _ => {
                            info!("Turning Buzzer off");
                            *mode = BuzzerMode::Off
                        }
                    }
                }
            }
            AsyncCmd::L { force, pulse } => {
                let mut motor_unlocked = MOTOR_MTX.lock().await;
                if let Some(motor) = motor_unlocked.as_mut() {
                    motor
                        .drive(
                            RingPosition::Locked,
                            if pulse { 100 } else { MOTOR_DRIVE_DUR_MS },
                            force,
                            MOTOR_DRIVE_CURR_MA,
                        )
                        .await;
                }
            }
            AsyncCmd::U { force, pulse } => {
                let mut motor_unlocked = MOTOR_MTX.lock().await;
                if let Some(motor) = motor_unlocked.as_mut() {
                    motor
                        .drive(
                            RingPosition::Unlocked,
                            if pulse { 100 } else { MOTOR_DRIVE_DUR_MS },
                            force,
                            MOTOR_DRIVE_CURR_MA,
                        )
                        .await;
                }
            }
            AsyncCmd::SetLimits { limits } => {
                let sensor1_limits: SensorLimits;
                let sensor2_limits: SensorLimits;

                // helper closure to parse incoming limits from str to u16
                let parse = |i: usize| -> Result<u16, _> { limits[i].parse::<u16>() };

                // Try to convert the incoming strings to u16s and create sensor limit structs
                if let (Ok(o), Ok(u), Ok(a), Ok(un), Ok(o2), Ok(u2), Ok(a2), Ok(un2)) =
                    (parse(0), parse(1), parse(2), parse(3), parse(4), parse(5), parse(6), parse(7))
                {
                    sensor1_limits = SensorLimits::new(o, u, a, un);
                    sensor2_limits = SensorLimits::new(o2, u2, a2, un2);
                } else {
                    error!("Error converting args to u16. Please try again");
                    continue;
                }

                fn u16_to_2u8(b: u16) -> [u8; 2] {
                    // create two u8s from a u16
                    [(b >> 8) as u8, b as u8]
                }

                // copy limits into a buffer to be stored into flash
                let mut fbuf = [0u8; (SENSOR_LIMIT_SECTOR_SIZE / 8) as usize];

                fbuf[0..2].copy_from_slice(&u16_to_2u8(sensor1_limits.over));
                fbuf[2..4].copy_from_slice(&u16_to_2u8(sensor1_limits.under));
                fbuf[4..6].copy_from_slice(&u16_to_2u8(sensor1_limits.active));
                fbuf[6..8].copy_from_slice(&u16_to_2u8(sensor1_limits.unactive));
                fbuf[8..10].copy_from_slice(&u16_to_2u8(sensor2_limits.over));
                fbuf[10..12].copy_from_slice(&u16_to_2u8(sensor2_limits.under));
                fbuf[12..14].copy_from_slice(&u16_to_2u8(sensor2_limits.active));
                fbuf[14..16].copy_from_slice(&u16_to_2u8(sensor2_limits.unactive));

                let mut flash_unlocked = FLASH_MTX.lock().await;
                if let Some(flash) = flash_unlocked.as_mut() {
                    // Sector must be erased before writing or SEQ err will be thrown
                    if let Err(e) = flash.blocking_erase(
                        SENSOR_LIMIT_SECTOR_OFFSET,
                        SENSOR_LIMIT_SECTOR_OFFSET + SENSOR_LIMIT_SECTOR_SIZE,
                    ) {
                        error!("Error erasing memory: {}", e);
                    }
                    if let Err(e) = flash.blocking_write(SENSOR_LIMIT_SECTOR_OFFSET, &fbuf) {
                        error!("Error writing sensor limits to memory: {}", e);
                    }
                }

                let mut ring_unlocked = RING_MTX.lock().await;
                if let Some(ring) = ring_unlocked.as_mut() {
                    ring.sensor1_limits = sensor1_limits;
                    ring.sensor2_limits = sensor2_limits;
                }
            }
            AsyncCmd::Erase => {
                let mut motor_unlocked = MOTOR_MTX.lock().await;
                if let Some(motor) = motor_unlocked.as_mut() {
                    motor.erase_actuation_data().await;
                }
            }
        }
    }
}
