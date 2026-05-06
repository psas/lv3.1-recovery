/* Sender board specific cli commands code */

use crate::{
    buzzer::{BuzzerMode, BUZZER_MODE_MTX},
    can::{DROGUE_DEPLOY_ID, MAIN_DEPLOY_ID},
    sender::can::send_deploy_msg,
};
use defmt::{error, info, Format};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, channel::Channel};
use embedded_cli::Command;

pub static ASYNC_CMD_CHANNEL: Channel<CriticalSectionRawMutex, SenderCmd, 1> = Channel::new();

#[derive(Debug, Command, Format, Clone, Copy)]
pub enum SenderCmd {
    /// Print internal state
    State,
    /// Send drogue release CAN message
    Drogue,
    /// Send main release CAN message
    Main,
    /// Toggle the Rocket Ready signal
    Rr,
    /// Print current battery voltage
    Batt,
    /// Toggle periodic beep
    Beep,
    /// Print info about the current firmware version
    Version,
}

#[embassy_executor::task]
pub async fn async_cmd_handler() {
    /* This task is more or less a hack to get the sync embedded-cli-rs crate to play nice with our
     * async runtime. It will await commands coming in on the channel and dispatch any async code
     * needed to handle said command.
     */
    loop {
        match ASYNC_CMD_CHANNEL.receive().await {
            SenderCmd::Drogue => {
                if let Err(e) = send_deploy_msg(DROGUE_DEPLOY_ID).await {
                    error!("Error sending deploy msg: {}", e);
                    continue;
                };
            }
            SenderCmd::Main => {
                if let Err(e) = (MAIN_DEPLOY_ID.await) {
                    error!("Error sending deploy msg: {}", e);
                    continue;
                }
            }
            SenderCmd::Beep => {
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
            _ => {
                error!("non-async cli command send to async handler");
            }
        }
    }
}
