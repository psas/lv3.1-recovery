/* Sender board specifc CAN code */

use core::sync::atomic::{AtomicBool, Ordering};

use defmt::{error, info};
use embassy_stm32::can::{
    enums::FrameCreateError, frame::Header, BufferedCanRx, Frame, Id, StandardId,
};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, signal::Signal};
use embassy_time::Timer;

use crate::can::{
    CanTxChannelMsg, CAN_BUF_SIZE, CAN_MTX, CAN_TX_CHANNEL, DROGUE_ACKNOWLEDGE_ID,
    DROGUE_DEPLOY_ID, DROGUE_HEARTBEAT_ID, MAIN_ACKNOWLEDGE_ID, MAIN_DEPLOY_ID, MAIN_HEARTBEAT_ID,
    SENDER_HEARTBEAT_ID,
};

#[derive(Clone, Copy, Default)]
pub struct CanState {
    pub drogue_ready: bool,
    pub drogue_last_seen: u64,
    pub main_ready: bool,
    pub main_last_seen: u64,
}

pub static MAIN_ACKNOWLEDGE: AtomicBool = AtomicBool::new(false);
pub static DROGUE_ACKNOWLEDGE: AtomicBool = AtomicBool::new(false);
pub static CAN_SIGNAL: Signal<CriticalSectionRawMutex, CanState> = Signal::new();

#[embassy_executor::task]
pub async fn can_reader(can_rx: BufferedCanRx<'static, CAN_BUF_SIZE>) -> () {
    /*
    Processes incoming CAN messages:
    - Updates parachute board status and timestamps from status messages
    - Records deployment acknowledgments from parachute boards
    - Tracks communication health with parachute boards
    */

    let rdr = can_rx.reader();
    let mut prev_main_ready: u8 = 0;
    let mut prev_drogue_ready: u8 = 0;
    let mut state = CanState::default();
    loop {
        match rdr.receive().await {
            Ok(envelope) => match envelope.frame.id() {
                Id::Standard(id) if id.as_raw() == MAIN_HEARTBEAT_ID => {
                    let status = envelope.frame.data()[5];
                    state.main_last_seen = envelope.ts.as_millis();
                    state.main_ready = status > 0;
                    if status != prev_main_ready {
                        info!("Main status changed to {}", status);
                    }
                    prev_main_ready = status;
                }
                Id::Standard(id) if id.as_raw() == MAIN_ACKNOWLEDGE_ID => {
                    MAIN_ACKNOWLEDGE.store(true, core::sync::atomic::Ordering::Relaxed);
                }
                Id::Standard(id) if id.as_raw() == DROGUE_HEARTBEAT_ID => {
                    let status = envelope.frame.data()[5];
                    state.drogue_last_seen = envelope.ts.as_millis();
                    state.drogue_ready = status > 0;
                    if status != prev_drogue_ready {
                        info!("Drogue status changed to {}", status);
                    }
                    prev_drogue_ready = status;
                }
                Id::Standard(id) if id.as_raw() == DROGUE_ACKNOWLEDGE_ID => {
                    DROGUE_ACKNOWLEDGE.store(true, core::sync::atomic::Ordering::Relaxed);
                }
                _ => {}
            },
            Err(e) => {
                error!("CAN Read Error: {}", e);
                let mut can_unlocked = CAN_MTX.lock().await;
                if let Some(can) = can_unlocked.as_mut() {
                    can.sleep().await;
                }
            }
        }
        CAN_SIGNAL.signal(state);
    }
}

pub async fn send_deploy_msg(can_id: u16) -> Result<(), FrameCreateError> {
    if let Some(id) = StandardId::new(can_id) {
        let header = Header::new(Id::Standard(id), 1, false);
        let frame = Frame::new(header, &[1; 0])?;

        match can_id {
            DROGUE_DEPLOY_ID => {
                info!("Releasing drogue");
                while !DROGUE_ACKNOWLEDGE.load(Ordering::Relaxed) {
                    let msg = CanTxChannelMsg::new(true, frame);
                    CAN_TX_CHANNEL.send(msg).await;
                    Timer::after_millis(100).await;
                }
            }
            MAIN_DEPLOY_ID => {
                info!("Releasing main");
                while !MAIN_ACKNOWLEDGE.load(Ordering::Relaxed) {
                    let msg = CanTxChannelMsg::new(true, frame);
                    CAN_TX_CHANNEL.send(msg).await;
                    Timer::after_millis(100).await;
                }
            }
            _ => {}
        }
        Ok(())
    } else {
        // This really isnt a frame create error, it's that Id::new returned None.
        // However this should never happen as we should always be passing valid CAN IDs right?
        // TODO: Custom err type?
        error!("unable to create valid CAN ID from provided value: {}", can_id);
        Err(FrameCreateError::InvalidCanId)
    }
}

pub struct HeartbeatContext {
    sender_state: u8,
    batt_read: u8,
    shore_pow_on: bool,
    main_ready: bool,
    drogue_ready: bool,
    can_bus_ok: bool,
    rocket_ready: bool,
}

impl HeartbeatContext {
    pub fn new(
        sender_state: u8,
        batt_read: u8,
        shore_pow_on: bool,
        can_bus_ok: bool,
        main_ready: bool,
        drogue_ready: bool,
        rocket_ready: bool,
    ) -> Self {
        Self {
            sender_state,
            batt_read,
            shore_pow_on,
            main_ready,
            drogue_ready,
            can_bus_ok,
            rocket_ready,
        }
    }
}

pub async fn send_heartbeat(ctx: HeartbeatContext) -> Result<(), FrameCreateError> {
    let ers_ready = ctx.can_bus_ok && ctx.drogue_ready && ctx.main_ready;
    let status_buf = [
        ctx.sender_state,
        ctx.batt_read,
        (ctx.batt_read > 99) as u8,
        ctx.shore_pow_on as u8,
        ctx.can_bus_ok as u8,
        ers_ready as u8,
        ctx.rocket_ready as u8,
        0,
    ];

    if let Some(id) = StandardId::new(SENDER_HEARTBEAT_ID) {
        // This will always be Some as the ID should never change and is certainly valid
        let header = Header::new(Id::Standard(id), 8, false);
        let frame = Frame::new(header, &status_buf)?;

        let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);

        CAN_TX_CHANNEL.send(msg).await;
        Ok(())
    } else {
        // This really isnt a frame create error, it's that Id::new returned None.
        // However this should never happen as we should always be passing valid CAN IDs right?
        // TODO: Custom err type?
        error!("unable to create valid CAN ID from provided value: {}", SENDER_HEARTBEAT_ID);
        Err(FrameCreateError::InvalidCanId)
    }
}
