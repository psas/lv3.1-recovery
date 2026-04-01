use defmt::*;
use embassy_stm32::can::{BufferedCanTx, Can, Frame, StandardId};
use embassy_sync::{
    blocking_mutex::raw::{CriticalSectionRawMutex, ThreadModeRawMutex},
    channel::Channel,
    mutex::Mutex,
};
use embassy_time::Timer;
use static_cell::StaticCell;

pub const CAN_BITRATE: u32 = 1_000_000;
pub const DROGUE_DEPLOY_ID: u16 = 0x100;
pub const DROGUE_HEARTBEAT_ID: u16 = 0x710;
pub const DROGUE_ACKNOWLEDGE_ID: u16 = 0x101;
pub const MAIN_DEPLOY_ID: u16 = 0x200;
pub const MAIN_HEARTBEAT_ID: u16 = 0x720;
pub const MAIN_ACKNOWLEDGE_ID: u16 = 0x201;
pub const SENDER_HEARTBEAT_ID: u16 = 0x700;

pub const CAN_BUF_SIZE: usize = 64;
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
    let tx_frame = Frame::new_data(unwrap!(StandardId::new(123 as _)), &[123]).unwrap();
    can.write(&tx_frame).await;
    loop {
        let envelope = can.read().await.unwrap();
        can.write(&envelope.frame).await;
        Timer::after_millis(1000).await;
    }
}

#[embassy_executor::task]
pub async fn can_writer(can_tx: BufferedCanTx<'static, CAN_BUF_SIZE>) -> () {
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
