use defmt::error;
use embassy_stm32::can::{frame::Header, BufferedCanRx, Frame, Id, StandardId};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, signal::Signal};

use crate::{
    can::{
        CanTxChannelMsg, CAN_BUF_SIZE, CAN_MTX, CAN_TX_CHANNEL, DROGUE_DEPLOY_ID, MAIN_DEPLOY_ID,
        SENDER_HEARTBEAT_ID,
    },
    parachute::{
        motor::{MOTOR_DRIVE_CURR_MA, MOTOR_DRIVE_DUR_MS, MOTOR_MTX},
        ring::RingPosition,
    },
};

pub static CAN_SIGNAL: Signal<CriticalSectionRawMutex, u64> = Signal::new();

#[embassy_executor::task]
pub async fn can_reader(can_rx: BufferedCanRx<'static, CAN_BUF_SIZE>) -> () {
    let rdr = can_rx.reader();
    loop {
        match rdr.receive().await {
            Ok(envelope) => match envelope.frame.id() {
                Id::Standard(id) if id.as_raw() == DROGUE_DEPLOY_ID => {
                    #[cfg(drogue)]
                    {
                        use crate::can::DROGUE_ACKNOWLEDGE_ID;
                        {
                            let mut motor_unlocked = MOTOR_MTX.lock().await;
                            if let Some(motor) = motor_unlocked.as_mut() {
                                motor
                                    .drive(
                                        RingPosition::Unlocked,
                                        MOTOR_DRIVE_DUR_MS,
                                        false,
                                        MOTOR_DRIVE_CURR_MA,
                                    )
                                    .await;
                            }
                        }
                        let frame =
                            Frame::new_data(StandardId::new(DROGUE_ACKNOWLEDGE_ID).unwrap(), &[1])
                                .unwrap();
                        let acknowledge_msg = CanTxChannelMsg::new(true, frame);
                        CAN_TX_CHANNEL.send(acknowledge_msg).await;
                    }
                }
                Id::Standard(id) if id.as_raw() == MAIN_DEPLOY_ID => {
                    #[cfg(main)]
                    {
                        use crate::can::MAIN_ACKNOWLEDGE_ID;
                        {
                            let mut motor_unlocked = MOTOR_MTX.lock().await;
                            if let Some(motor) = motor_unlocked.as_mut() {
                                motor
                                    .drive(
                                        RingPosition::Unlocked,
                                        MOTOR_DRIVE_DUR_MS,
                                        false,
                                        MOTOR_DRIVE_CURR_MA,
                                    )
                                    .await;
                            }
                        }
                        let frame =
                            Frame::new_data(StandardId::new(MAIN_ACKNOWLEDGE_ID).unwrap(), &[1])
                                .unwrap();
                        let acknowledge_msg = CanTxChannelMsg::new(true, frame);
                        CAN_TX_CHANNEL.send(acknowledge_msg).await;
                    }
                }
                Id::Standard(id) if id.as_raw() == SENDER_HEARTBEAT_ID => {
                    CAN_SIGNAL.signal(envelope.ts.as_millis());
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
    }
}

pub struct HeartbeatCtx {
    ring_pos: RingPosition,
    batt_read: u8,
    shore_pow_on: bool,
    sender_ok: bool,
    ready: bool,
}

impl HeartbeatCtx {
    pub fn new(
        ring_pos: RingPosition,
        batt_read: u8,
        shore_pow_on: bool,
        sender_ok: bool,
        ready: bool,
    ) -> Self {
        Self { ring_pos, batt_read, shore_pow_on, sender_ok, ready }
    }
}

pub async fn send_heartbeat(ctx: HeartbeatCtx) {
    let ring_pos_u8: u8 = match ctx.ring_pos {
        RingPosition::Unlocked => 1,
        RingPosition::Inbetween => 2,
        RingPosition::Locked => 3,
        RingPosition::Error => 4,
    };

    let status_buf = [
        ring_pos_u8,
        ctx.batt_read,
        (ctx.batt_read > 99) as u8,
        ctx.shore_pow_on as u8,
        ctx.sender_ok as u8,
        ctx.ready as u8,
        0,
        0,
    ];

    #[cfg(main)]
    {
        use crate::can::MAIN_HEARTBEAT_ID;
        let id = StandardId::new(MAIN_HEARTBEAT_ID).unwrap();
        let header = Header::new(Id::Standard(id), 8, false);
        let frame = Frame::new(header, &status_buf).unwrap();
        let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);
        CAN_TX_CHANNEL.send(msg).await;
    }

    #[cfg(drogue)]
    {
        use crate::can::DROGUE_HEARTBEAT_ID;
        let id = StandardId::new(DROGUE_HEARTBEAT_ID).unwrap();
        let header = Header::new(Id::Standard(id), 8, false);
        let frame = Frame::new(header, &status_buf).unwrap();
        let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);
        CAN_TX_CHANNEL.send(msg).await;
    }
}
