/*
* This module is for project wide CAN related code. It defines several constants, creates a channel,
* defines the `CanTxChannelMsg` struct, and defines two tasks, `echo_can` and `can_writer`.⏎
*
* CAN IDs follow a numbering scheme:
* - **0x1XX**: Drogue parachute commands and acknowledgments
* - **0x2XX**: Main parachute commands and acknowledgments
* - **0x7XX**: System status and heartbeat messages
*/

use defmt::{error, info, unwrap};
use embassy_stm32::can::{BufferedCanTx, Can, Frame, StandardId};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    channel::Channel,
    mutex::Mutex,
};
use embassy_time::Timer;
use static_cell::StaticCell;

pub const CAN_BITRATE: u32 = 1_000_000;
pub const DROGUE_DEPLOY_ID: u16 = 0x100; // drogue pcb ID for deploying the drogue parachute
pub const DROGUE_HEARTBEAT_ID: u16 = 0x710;
pub const DROGUE_ACKNOWLEDGE_ID: u16 = 0x101; // drouge pcb ID to confirm chute deployment
pub const MAIN_DEPLOY_ID: u16 = 0x200; // main pcb ID for deploying the main parachute
pub const MAIN_HEARTBEAT_ID: u16 = 0x720;
pub const MAIN_ACKNOWLEDGE_ID: u16 = 0x201; // main pcb ID to confirm chute deployment
pub const SENDER_HEARTBEAT_ID: u16 = 0x700;

pub const CAN_BUF_SIZE: usize = 64; // size of the buffers used in the CAN driver

pub static CAN_RX_BUF: StaticCell<embassy_stm32::can::RxBuf<CAN_BUF_SIZE>> = StaticCell::new();
pub static CAN_TX_BUF: StaticCell<embassy_stm32::can::TxBuf<CAN_BUF_SIZE>> = StaticCell::new();
pub static CAN_TX_CHANNEL: Channel<CriticalSectionRawMutex, CanTxChannelMsg, 10> = Channel::new();
pub static CAN_MTX: Mutex<ThreadModeRawMutex, Option<Can>> = Mutex::new(None);

pub struct CanTxChannelMsg {
    pub blocking: bool,
    pub frame: Frame,
}

impl CanTxChannelMsg {
    pub fn new(blocking: bool, frame: Frame) -> Self {
        Self { blocking, frame }
    }
}

#[embassy_executor::task]
pub async fn echo_can(mut can: Can<'static>) -> () {
    /* Debug task that will simply write a message to the bus, and then enter a loop that waits
     * for a response which it then echoes back to the bus again. To be used with `echo_can.py`.
     */
    if let Some(id) = StandardId::new(123 as _) {
        let tx_frame = unwrap!(Frame::new_data(id, &[123]));
        can.write(&tx_frame).await;
        loop {
            let envelope = unwrap!(can.read().await);
            can.write(&envelope.frame).await;
            Timer::after_millis(1000).await;
        }
    }
}

#[embassy_executor::task]
pub async fn can_writer(can_tx: BufferedCanTx<'static, CAN_BUF_SIZE>) -> () {
    /* Handles writing messages to the CAN bus for all boards. The channel used by this task only
     * accepts a CanTxChannelMsg, which is a simple wrapper around Embassy's CAN Frame, that tells
     * the writer task if it should write the message with a blocking write, or if it should only
     * try to write the message and fail with an error if it can't.
     */
    let mut wrt = can_tx.writer();
    loop {
        let frame = CAN_TX_CHANNEL.receive().await;
        if frame.blocking {
            info!("writing to CAN");
            wrt.write(frame.frame).await;
        } else if let Err(e) = wrt.try_write(frame.frame) {
            error!("Could not send CAN message: {}", e);
            let mut can_unlocked = CAN_MTX.lock().await;
            if let Some(can) = can_unlocked.as_mut() {
                // Try to recover from bus_off mode
                can.modify_config();
                can.enable().await;
            }
        }
    }
}
