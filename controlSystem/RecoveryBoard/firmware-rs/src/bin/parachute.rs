#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
#[cfg(feature = "main")]
use embassy_stm32::can::{Frame, StandardId};
use embassy_stm32::{
    adc::{Adc, InterruptHandler, SampleTime},
    bind_interrupts,
    can::{
        frame::Header, Can, CanRx, Fifo, Id, Rx0InterruptHandler, Rx1InterruptHandler,
        SceInterruptHandler, TxInterruptHandler,
    },
    gpio::{Input, Level, Output, OutputType, Pull, Speed},
    peripherals::{ADC1, CAN, USART2},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{BufferedUart, Config as UartConfig, DataBits, Parity, StopBits},
};
use embassy_stm32::{can::filter::Mask32, dac::Dac, usart::BufferedInterruptHandler};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex};

use embassy_time::{Instant, Timer};
use embedded_io_async::Write;
#[cfg(feature = "main")]
use firmware_rs::{
    adc::{read_battery_from_ref, ADC_MTX, BATT_READ_WATCH},
    buzzer::{BuzzerMode, BUZZER_MODE_MTX},
    can::{
        can_writer, CanTxChannelMsg, CAN_BITRATE, CAN_TX_CHANNEL, DROGUE_ACKNOWLEDGE_ID,
        DROGUE_DEPLOY_ID, MAIN_ACKNOWLEDGE_ID, MAIN_DEPLOY_ID,
    },
    motor::{Motor, MotorType},
    ring::{read_pos_sensor, Ring, RingPosition, RingType, RING_POSITION_WATCH},
    types::*,
    uart::{IO, UART_BUF_SIZE, UART_RX_BUF_CELL, UART_TX_BUF_CELL},
};
use noline::builder::EditorBuilder;
use {defmt_rtt as _, panic_probe as _};

const MOTOR_DRIVE_DUR_MS: u64 = 1000;
const MOTOR_DRIVE_CURR_MA: u16 = 1000;

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => InterruptHandler<ADC1>; });
bind_interrupts!(struct UsartIrqs { USART2 => BufferedInterruptHandler<USART2>; });

#[derive(Default)]
pub struct ChuteState {
    pub shore_power_status: bool,
    pub sender_last_seen: u64,
}

#[derive(Debug)]
pub enum ChuteStateField {
    ShorePowerStatus(bool),
    SenderLastSeen(u64),
}

pub struct ChuteStateIter<'a> {
    state_fields: &'a ChuteState,
    index: usize,
}

impl ChuteState {
    pub fn iter(&self) -> ChuteStateIter<'_> {
        ChuteStateIter { state_fields: self, index: 0 }
    }
}

impl<'a> Iterator for ChuteStateIter<'a> {
    type Item = ChuteStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(ChuteStateField::ShorePowerStatus(self.state_fields.shore_power_status)),
            1 => Some(ChuteStateField::SenderLastSeen(self.state_fields.sender_last_seen)),
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
        match *self {
            Self::ShorePowerStatus(val) => {
                core::write!(f, "Shore Power: {}", if val { "ON" } else { "OFF" })
            }
            Self::SenderLastSeen(val) => core::write!(f, "Sender last seen: {}ms", val),
        }
    }
}

async fn set_state(update: ChuteStateField) {
    let mut unlocked = SYSTEM_STATE_MTX.lock().await;
    if let Some(state) = unlocked.as_mut() {
        match update {
            ChuteStateField::ShorePowerStatus(val) => state.shore_power_status = val,
            ChuteStateField::SenderLastSeen(val) => state.sender_last_seen = val,
        }
    }
}

static UMB_ON_MTX: UmbOnType = Mutex::new(None);
static SYSTEM_STATE_MTX: Mutex<ThreadModeRawMutex, Option<ChuteState>> = Mutex::new(None);
static RING_MTX: RingType = Mutex::new(None);
static MOTOR_MTX: MotorType = Mutex::new(None);

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let umb_on = Input::new(p.PA8, Pull::Up);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);

    // Set up PWM driver
    let buzz_pin = PwmPin::new(p.PB15, OutputType::PushPull);
    let _pwm = SimplePwm::new(
        p.TIM15,
        None,
        Some(buzz_pin),
        None,
        None,
        Hertz(440),
        CountingMode::EdgeAlignedUp,
    );

    let buzzer_mode = BuzzerMode::Off;

    // Set up CAN driver
    let mut can = Can::new(p.CAN, p.PA11, p.PA12, CanIrqs);
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());
    can.modify_config().set_bitrate(CAN_BITRATE).set_loopback(false).set_silent(false);

    // Set up ADC driver
    let mut adc = Adc::new(p.ADC1, AdcIrqs);
    adc.set_sample_time(SampleTime::CYCLES239_5);
    adc.set_resolution(embassy_stm32::adc::Resolution::BITS12);

    // Set up UART driver
    let mut uart_config = UartConfig::default();
    uart_config.baudrate = 115200;
    uart_config.parity = Parity::ParityNone;
    uart_config.data_bits = DataBits::DataBits8;
    uart_config.stop_bits = StopBits::STOP1;
    let uart = BufferedUart::new(
        p.USART2,
        p.PA3,
        p.PA2,
        UART_TX_BUF_CELL.take(),
        UART_RX_BUF_CELL.take(),
        UsartIrqs,
        uart_config,
    )
    .expect("Uart Config Error");

    let dac = Dac::new(p.DAC1, p.DMA1_CH3, p.DMA1_CH4, p.PA4, p.PA5);
    let sys_state = ChuteState::default();
    let motor = Motor::new(p.PB4, p.PB5, p.PB6, p.PB7, dac, &RING_POSITION_WATCH);
    let ring = Ring::new(p.PA0, p.PA1, &ADC_MTX);

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(BUZZER_MODE_MTX.lock().await) = Some(buzzer_mode);
        *(ADC_MTX.lock().await) = Some(adc);
        *(UMB_ON_MTX.lock().await) = Some(umb_on);
        *(SYSTEM_STATE_MTX.lock().await) = Some(sys_state);
        *(RING_MTX.lock().await) = Some(ring);
        *(MOTOR_MTX.lock().await) = Some(motor);
    }

    // unwrap!(spawner.spawn(active_beep(pwm, None)));
    unwrap!(spawner.spawn(cli(uart)));
    unwrap!(spawner.spawn(read_battery_from_ref(&ADC_MTX, p.PB0)));
    unwrap!(spawner.spawn(read_pos_sensor(&RING_MTX)));

    // enable can at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    unwrap!(spawner.spawn(can_writer(can_tx, &CAN_TX_CHANNEL)));
    unwrap!(spawner.spawn(can_reader(can_rx, can)));
    unwrap!(spawner.spawn(parachute_heartbeat()));

    // Keep main from returning. Needed for can_tx/can_rx or they get dropped
    core::future::pending::<()>().await;
}

#[embassy_executor::task]
pub async fn cli(uart: BufferedUart<'static>) {
    let prompt = "> ";
    let mut io = IO::new(uart);
    let mut buffer = [0; UART_BUF_SIZE];
    let mut history = [0; UART_BUF_SIZE];
    loop {
        let mut editor = EditorBuilder::from_slice(&mut buffer)
            .with_slice_history(&mut history)
            .build_async(&mut io)
            .await
            .unwrap();

        while let Ok(line) = editor.readline(prompt, &mut io).await {
            // WARN: Passing more than 5 args panics
            let args: heapless::Vec<&str, 5> = line.split_whitespace().collect();

            if args.len() > 3usize {
                error!("Only two arguments allowed");
                continue;
            }

            if args.is_empty() {
                error!("args empty");
                continue;
            }

            match args[0] {
                "help" => {
                    let lines = [
                        "help: Display this message.\r\n\n",
                        "state: Print internal state.\r\n\n",
                        "l: Move the ring towards the lock position.\r\n",
                        " --force: Ignore sensor readings, continue until timeout.\r\n",
                        " --pulse: Do a 100ms step instead of a full swing.\r\n\n",
                        "u: Move the ring towards the unlocked position.\r\n",
                        " --force: Ignore sensor readings, continue until timeout.\r\n",
                        " --pulse: Do a 100ms step instead of a full swing.\r\n\n",
                        "pos: Print the current sensor readings and ring state.\r\n",
                        " --poll: Print the sensor value and ring state every second.\r\n\n",
                    ];
                    for line in lines {
                        io.write(line.as_bytes()).await.unwrap();
                    }
                    io.flush().await.unwrap();
                }
                "state" => {
                    let mut buf = [0u8; 64];
                    let mut state_unlocked = SYSTEM_STATE_MTX.lock().await;
                    if let Some(state) = state_unlocked.as_mut() {
                        let time_now = Instant::now().as_millis();
                        let ts = format_no_std::show(
                            &mut buf,
                            format_args!("Current time: {}ms\r\n", time_now),
                        )
                        .unwrap();
                        io.write(ts.as_bytes()).await.unwrap();
                        for field in state.iter() {
                            let s = format_no_std::show(&mut buf, format_args!("{}\r\n", field))
                                .unwrap();
                            io.write(s.as_bytes()).await.unwrap();
                        }
                    }
                }
                "batt" => {
                    let mut buf = [0u8; 16];

                    let batt_read = BATT_READ_WATCH
                        .receiver()
                        .expect("Could not get batt_read receiver")
                        .changed()
                        .await;

                    let s =
                        format_no_std::show(&mut buf, format_args!("{}\r\n", batt_read)).unwrap();

                    io.write(s.as_bytes()).await.unwrap();
                }
                "l" => {
                    let mut motor_unlocked = MOTOR_MTX.lock().await;
                    if let Some(motor) = motor_unlocked.as_mut() {
                        motor
                            .drive(
                                RingPosition::Locked,
                                if args.contains(&"--pulse") {
                                    100
                                } else {
                                    MOTOR_DRIVE_DUR_MS
                                },
                                args.contains(&"--force"),
                                MOTOR_DRIVE_CURR_MA,
                            )
                            .await;
                    }
                }
                "u" => {
                    let mut motor_unlocked = MOTOR_MTX.lock().await;
                    if let Some(motor) = motor_unlocked.as_mut() {
                        motor
                            .drive(
                                RingPosition::Unlocked,
                                if args.contains(&"--pulse") {
                                    100
                                } else {
                                    MOTOR_DRIVE_DUR_MS
                                },
                                args.contains(&"--force"),
                                MOTOR_DRIVE_CURR_MA,
                            )
                            .await;
                    }
                }
                "pos" => {
                    // TODO: implement
                    io.write(b"Not yet implemented\r\n").await.unwrap();
                }
                _ => {
                    io.write(b"Invalid command\r\n").await.unwrap();
                }
            }
        }
    }
}

#[embassy_executor::task]
async fn can_reader(mut can_rx: CanRx<'static>, mut can: Can<'static>) -> () {
    loop {
        match can_rx.read().await {
            Ok(envelope) => match envelope.frame.id() {
                Id::Standard(id) if id.as_raw() == DROGUE_DEPLOY_ID => {
                    let frame =
                        Frame::new_data(StandardId::new(DROGUE_ACKNOWLEDGE_ID).unwrap(), &[1])
                            .unwrap();
                    let acknowledge_msg = CanTxChannelMsg::new(true, frame);
                    CAN_TX_CHANNEL.send(acknowledge_msg).await;

                    #[cfg(feature = "drogue")]
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
                }
                Id::Standard(id) if id.as_raw() == MAIN_DEPLOY_ID => {
                    #[cfg(feature = "main")]
                    let frame =
                        Frame::new_data(StandardId::new(MAIN_ACKNOWLEDGE_ID).unwrap(), &[1])
                            .unwrap();
                    let acknowledge_msg = CanTxChannelMsg::new(true, frame);
                    CAN_TX_CHANNEL.send(acknowledge_msg).await;
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
                }
                _ => {}
            },

            Err(e) => {
                error!("CAN Read Error: {}", e);
                can.sleep().await;
            }
        }
    }
}

#[embassy_executor::task]
async fn parachute_heartbeat() -> () {
    let mut prev_ready: u8 = 0;
    let mut prev_batt_ok: u8 = 0;
    let mut prev_sender_status: u8 = 0;
    let mut prev_shore_pow_status: u8 = 0;

    let mut ring_pos_rcvr =
        RING_POSITION_WATCH.receiver().expect("Could not get ring pos receiver for heartbeat task");

    let mut batt_read_rcvr =
        BATT_READ_WATCH.receiver().expect("Could not get batt read receiver for heartbeat task");

    loop {
        let time_now = Instant::now().as_millis();
        let sender_last_seen: u64;

        {
            let mut state_unlocked = SYSTEM_STATE_MTX.lock().await;
            if let Some(state) = state_unlocked.as_mut() {
                sender_last_seen = state.sender_last_seen;
            } else {
                error!("Could not read state for heartbeat");
                continue;
            }
        }

        let ring_position = ring_pos_rcvr.changed().await;
        let ring_pos_u8: u8 = match ring_position {
            RingPosition::Unlocked => 0,
            RingPosition::Inbetween => 1,
            RingPosition::Locked => 2,
            _ => 3,
        };

        let batt_read = batt_read_rcvr.changed().await;
        let batt_ok: u8 = (batt_read > 99) as u8;

        let sender_status: u8 = ((time_now - sender_last_seen) < 2000) as u8;

        {
            let mut umb_on_unlocked = UMB_ON_MTX.lock().await;
            if let Some(umb_on_ref) = umb_on_unlocked.as_mut() {
                let shore_pow_status = umb_on_ref.is_low() as u8;

                let ready = (ring_pos_u8 == 2
                    && shore_pow_status == 0
                    && batt_ok == 1
                    && sender_status == 1) as u8;

                {
                    let mut buzz_mode_unlocked = BUZZER_MODE_MTX.lock().await;
                    if let Some(mode) = buzz_mode_unlocked.as_mut() {
                        match mode {
                            BuzzerMode::Off => {}
                            _ => {
                                if ready > 0 {
                                    *mode = BuzzerMode::High;
                                } else {
                                    *mode = BuzzerMode::Low;
                                }
                            }
                        }
                    }
                }

                let status_buf = [
                    ring_pos_u8,
                    batt_read,
                    batt_ok,
                    shore_pow_status,
                    sender_status,
                    ready,
                    0,
                    0,
                ];

                #[cfg(feature = "main")]
                {
                    use firmware_rs::can::MAIN_STATUS_ID;
                    let id = StandardId::new(MAIN_STATUS_ID).unwrap();
                    let header = Header::new(Id::Standard(id), 8, false);
                    let frame = Frame::new(header, &status_buf).unwrap();
                    let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);
                    CAN_TX_CHANNEL.send(msg).await;
                }

                #[cfg(feature = "drogue")]
                {
                    use firmware_rs::can::DROGUE_STATUS_ID;
                    let id = StandardId::new(DROGUE_STATUS_ID).unwrap();
                    let header = Header::new(Id::Standard(id), 8, false);
                    let frame = Frame::new(header, &status_buf).unwrap();
                    let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);
                    CAN_TX_CHANNEL.send(msg).await;
                }

                if batt_ok != prev_batt_ok {
                    info!("Battery ok changed to {}", batt_ok);
                }

                if ready != prev_ready {
                    info!("Ready status changed to {}", ready);
                }

                if shore_pow_status != prev_shore_pow_status {
                    info!("Shore power status changed to {}", shore_pow_status);
                }
                if sender_status != prev_sender_status {
                    info!("Sender status changed to {}", sender_status);
                }

                prev_sender_status = sender_status;
                prev_shore_pow_status = shore_pow_status;
                prev_ready = ready;
                prev_batt_ok = batt_ok;
            }
        }
        Timer::after_secs(1).await;
    }
}
