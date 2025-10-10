#![no_std]
#![no_main]

use core::sync::atomic::AtomicBool;

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler, SampleTime},
    bind_interrupts,
    can::{
        filter::Mask32,
        frame::Header,
        Can, CanRx, Fifo, Frame,
        Id::{self, Standard},
        Rx0InterruptHandler, Rx1InterruptHandler, SceInterruptHandler, StandardId,
        TxInterruptHandler,
    },
    exti::ExtiInput,
    gpio::{Input, Level, Output, OutputType, Pull, Speed},
    peripherals::{ADC1, CAN, USART2},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{
        BufferedInterruptHandler, BufferedUart, Config as UartConfig, DataBits, Parity, StopBits,
    },
};
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, mutex::Mutex};
use embassy_time::{Instant, Timer};
use embedded_io_async::Write;
use firmware_rs::{
    adc::{read_battery, BATT_READ_WATCH},
    buzzer::{active_beep, BuzzerMode, BUZZER_MODE_MTX},
    can::{
        can_writer, CanTxChannelMsg, CAN_BITRATE, CAN_TX_CHANNEL, DROGUE_ACKNOWLEDGE_ID,
        DROGUE_DEPLOY_ID, DROGUE_STATUS_ID, MAIN_ACKNOWLEDGE_ID, MAIN_DEPLOY_ID, MAIN_STATUS_ID,
        TELEMETRUM_HEARTBEAT_ID,
    },
    types::*,
    uart::{IO, UART_BUF_SIZE, UART_RX_BUF_CELL, UART_TX_BUF_CELL},
};
use noline::builder::EditorBuilder;
use {defmt_rtt as _, panic_probe as _};

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => InterruptHandler<ADC1>; });
bind_interrupts!(struct UsartIrqs { USART2 => BufferedInterruptHandler<USART2>; });

static UMB_ON_MTX: UmbOnType = Mutex::new(None);
static SYSTEM_STATE_MTX: Mutex<ThreadModeRawMutex, Option<SenderState>> = Mutex::new(None);

static MAIN_ACKNOWLEDGE: AtomicBool = AtomicBool::new(false);
static DROGUE_ACKNOWLEDGE: AtomicBool = AtomicBool::new(false);

#[derive(Default)]
pub struct SenderState {
    pub rocket_ready: bool,
    pub force_rocket_ready: bool,
    pub drogue_status: bool,
    pub main_status: bool,
    pub shore_power_status: bool,
    pub drogue_last_seen: u64,
    pub main_last_seen: u64,
    pub iso_main_last_seen: u64,
    pub iso_drogue_last_seen: u64,
}

#[derive(Debug)]
pub enum SenderStateField {
    RocketReady(bool),
    ForceRocketReady(bool),
    DrogueStatus(bool),
    MainStatus(bool),
    ShorePowerStatus(bool),
    DrogueLastSeen(u64),
    MainLastSeen(u64),
    IsoMainLastSeen(u64),
    IsoDrogueLastSeen(u64),
}

pub struct SenderStateIter<'a> {
    state_fields: &'a SenderState,
    index: usize,
}

impl SenderState {
    pub fn iter(&self) -> SenderStateIter<'_> {
        SenderStateIter { state_fields: self, index: 0 }
    }
}

impl<'a> Iterator for SenderStateIter<'a> {
    type Item = SenderStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(SenderStateField::RocketReady(self.state_fields.rocket_ready)),
            1 => Some(SenderStateField::ForceRocketReady(self.state_fields.force_rocket_ready)),
            2 => Some(SenderStateField::DrogueStatus(self.state_fields.drogue_status)),
            3 => Some(SenderStateField::MainStatus(self.state_fields.main_status)),
            4 => Some(SenderStateField::ShorePowerStatus(self.state_fields.shore_power_status)),
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
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match *self {
            Self::RocketReady(val) => {
                core::write!(f, "Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            Self::ForceRocketReady(val) => {
                core::write!(f, "Force Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            Self::DrogueStatus(val) => {
                core::write!(f, "Drogue Status: {}", if val { "OK" } else { "ERR" })
            }
            Self::MainStatus(val) => {
                core::write!(f, "Main Status: {}", if val { "OK" } else { "ERR" })
            }
            Self::ShorePowerStatus(val) => {
                core::write!(f, "Shore Power: {}", if val { "ON" } else { "OFF" })
            }
            Self::DrogueLastSeen(val) => core::write!(f, "Drogue last seen: {}ms", val),
            Self::MainLastSeen(val) => core::write!(f, "Main last seen: {}ms", val),
            Self::IsoDrogueLastSeen(val) => core::write!(f, "Iso drogue last seen: {}ms", val),
            Self::IsoMainLastSeen(val) => core::write!(f, "Iso main last seen: {}ms", val),
        }
    }
}

async fn set_state(update: SenderStateField) {
    let mut unlocked = SYSTEM_STATE_MTX.lock().await;
    if let Some(state) = unlocked.as_mut() {
        match update {
            SenderStateField::RocketReady(val) => state.rocket_ready = val,
            SenderStateField::ForceRocketReady(val) => state.force_rocket_ready = val,
            SenderStateField::DrogueStatus(val) => state.drogue_status = val,
            SenderStateField::MainStatus(val) => state.main_status = val,
            SenderStateField::ShorePowerStatus(val) => state.shore_power_status = val,
            SenderStateField::DrogueLastSeen(val) => state.drogue_last_seen = val,
            SenderStateField::MainLastSeen(val) => state.main_last_seen = val,
            SenderStateField::IsoMainLastSeen(val) => state.iso_main_last_seen = val,
            SenderStateField::IsoDrogueLastSeen(val) => state.iso_drogue_last_seen = val,
        }
    }
}

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let umb_on = Input::new(p.PA8, Pull::Up);
    let iso_main = ExtiInput::new(p.PA6, p.EXTI6, Pull::Down);
    let iso_drogue = ExtiInput::new(p.PA5, p.EXTI5, Pull::Down);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);
    let rocket_ready_pin = Output::new(p.PA7, Level::Low, Speed::Medium);

    // Set up PWM driver
    let buzz_pin = PwmPin::new(p.PB15, OutputType::PushPull);
    let pwm = SimplePwm::new(
        p.TIM15,
        None,
        Some(buzz_pin),
        None,
        None,
        Hertz(440),
        CountingMode::EdgeAlignedUp,
    );

    let buzzer_mode = BuzzerMode::Low;

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

    let sys_state: SenderState = SenderState::default();

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(BUZZER_MODE_MTX.lock().await) = Some(buzzer_mode);
        *(UMB_ON_MTX.lock().await) = Some(umb_on);
        *(SYSTEM_STATE_MTX.lock().await) = Some(sys_state);
    }

    unwrap!(spawner.spawn(active_beep(pwm, &BUZZER_MODE_MTX)));
    unwrap!(spawner.spawn(cli(uart)));
    unwrap!(spawner.spawn(read_battery(adc, p.PB0)));
    unwrap!(spawner.spawn(handle_iso_rising_edge(iso_drogue, DROGUE_DEPLOY_ID)));
    unwrap!(spawner.spawn(handle_iso_rising_edge(iso_main, MAIN_DEPLOY_ID)));
    unwrap!(spawner.spawn(telemetrum_heartbeat(rocket_ready_pin)));

    // enable at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    unwrap!(spawner.spawn(can_writer(can_tx, &CAN_TX_CHANNEL)));
    unwrap!(spawner.spawn(can_reader(can_rx, can)));

    // Keep main from returning. Needed for can_tx/can_rx or they get dropped
    core::future::pending::<()>().await;
}

async fn deploy(can_id: u16) {
    let id = unwrap!(StandardId::new(can_id));
    let header = Header::new(Id::Standard(id), 1, false);
    let frame = unwrap!(Frame::new(header, &[1; 0]));

    match can_id {
        DROGUE_DEPLOY_ID => {
            info!("Releasing drogue");
            while !DROGUE_ACKNOWLEDGE.load(core::sync::atomic::Ordering::Relaxed) {
                let msg = CanTxChannelMsg::new(true, frame);
                CAN_TX_CHANNEL.send(msg).await;
                Timer::after_millis(100).await;
            }
            info!("Drogue release acknowledged");
            DROGUE_ACKNOWLEDGE.store(false, core::sync::atomic::Ordering::Relaxed);
        }
        MAIN_DEPLOY_ID => {
            info!("Releasing main");
            while !MAIN_ACKNOWLEDGE.load(core::sync::atomic::Ordering::Relaxed) {
                let msg = CanTxChannelMsg::new(true, frame);
                CAN_TX_CHANNEL.send(msg).await;
                Timer::after_millis(100).await;
            }
            info!("Main release acknowledged");
            MAIN_ACKNOWLEDGE.store(false, core::sync::atomic::Ordering::Relaxed);
        }
        _ => {}
    }
}

#[embassy_executor::task]
async fn cli(uart: BufferedUart<'static>) {
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
            match line {
                "help" => {
                    let lines = [
                        "help: Display this message.\r\n\n",
                        "state: Print internal state.\r\n\n",
                        "drogue: Send drogue release CAN message.\r\n\n",
                        "main: Send main release CAN message.\r\n\n",
                        "rr: Toggle the Rocket Ready signal\r\n\n",
                        "batt: print current battery voltage.\r\n\n",
                        "beep: Toggle periodic beep.\r\n\n",
                        "version: Print info about the current firmware version\r\n\n",
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
                "drogue" => {
                    deploy(DROGUE_DEPLOY_ID).await;
                }
                "main" => {
                    deploy(MAIN_DEPLOY_ID).await;
                }
                "rr" => {
                    let toggled_rr: bool;
                    {
                        let mut state_unlocked = SYSTEM_STATE_MTX.lock().await;
                        if let Some(state) = state_unlocked.as_mut() {
                            toggled_rr = !state.force_rocket_ready;
                        } else {
                            toggled_rr = false;
                        }
                    }
                    set_state(SenderStateField::ForceRocketReady(toggled_rr)).await;
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
                "beep" => {
                    // toggle beeping
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
                "version" => {
                    // TODO: implement - print version details
                }
                _ => {
                    io.write(b"Invalid Command\r\n").await.unwrap();
                }
            }
        }
    }
}

#[embassy_executor::task]
async fn can_reader(mut can_rx: CanRx<'static>, mut can: Can<'static>) -> () {
    let mut prev_main_status: u8 = 0;
    let mut prev_drogue_status: u8 = 0;
    loop {
        match can_rx.read().await {
            Ok(envelope) => match envelope.frame.id() {
                Id::Standard(id) if id.as_raw() == MAIN_STATUS_ID => {
                    let status = envelope.frame.data()[0];
                    set_state(SenderStateField::MainLastSeen(envelope.ts.as_millis())).await;
                    set_state(SenderStateField::MainStatus(status > 0)).await;
                    if status != prev_main_status {
                        info!("Main status changed to {}", status);
                    }
                    prev_main_status = status;
                }
                Id::Standard(id) if id.as_raw() == MAIN_ACKNOWLEDGE_ID => {
                    MAIN_ACKNOWLEDGE.store(true, core::sync::atomic::Ordering::Relaxed);
                }
                Id::Standard(id) if id.as_raw() == DROGUE_STATUS_ID => {
                    let status = envelope.frame.data()[0];
                    set_state(SenderStateField::DrogueLastSeen(envelope.ts.as_millis())).await;
                    set_state(SenderStateField::DrogueStatus(status > 0)).await;
                    if status != prev_drogue_status {
                        info!("Drogue status changed to {}", status);
                    }
                    prev_drogue_status = status;
                }
                Id::Standard(id) if id.as_raw() == DROGUE_ACKNOWLEDGE_ID => {
                    DROGUE_ACKNOWLEDGE.store(true, core::sync::atomic::Ordering::Relaxed);
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

#[embassy_executor::task(pool_size = 2)]
async fn handle_iso_rising_edge(mut iso: ExtiInput<'static>, can_id: u16) -> () {
    loop {
        iso.wait_for_rising_edge().await;
        let time_now = Instant::now().as_millis();
        // telemetrum is signaling to deploy
        {
            let mut umb_on_unlocked = UMB_ON_MTX.lock().await;
            if let Some(umb_on_ref) = umb_on_unlocked.as_mut() {
                if umb_on_ref.is_high() {
                    deploy(can_id).await;
                }
            }
        }
        // record last deployment signal time
        match can_id {
            DROGUE_DEPLOY_ID => {
                set_state(SenderStateField::IsoDrogueLastSeen(time_now)).await;
            }
            MAIN_DEPLOY_ID => {
                set_state(SenderStateField::IsoMainLastSeen(time_now)).await;
            }
            _ => {}
        }
    }
}

#[embassy_executor::task]
async fn telemetrum_heartbeat(mut rr_pin: Output<'static>) -> () {
    let mut prev_main_ok: bool = false;
    let mut prev_drogue_ok: bool = false;
    let mut prev_batt_ok: u8 = 0;
    let mut prev_ers_status: u8 = 0;
    let mut prev_telemetrum_state: u8 = 0;
    let mut prev_shore_pow_status: u8 = 0;
    let mut prev_rocket_ready: u8 = 0;

    loop {
        let time_now = Instant::now().as_millis();

        let force_rocket_ready: bool;
        let main_status: bool;
        let drogue_status: bool;
        let drogue_last_seen: u64;
        let main_last_seen: u64;
        let iso_drogue_last_seen: u64;
        let iso_main_last_seen: u64;

        {
            let mut state_unlocked = SYSTEM_STATE_MTX.lock().await;
            if let Some(state) = state_unlocked.as_mut() {
                force_rocket_ready = state.force_rocket_ready;
                main_status = state.main_status;
                drogue_status = state.drogue_status;
                drogue_last_seen = state.drogue_last_seen;
                main_last_seen = state.main_last_seen;
                iso_drogue_last_seen = state.drogue_last_seen;
                iso_main_last_seen = state.iso_main_last_seen;
            } else {
                error!("Could not read state for telemetrum_heartbeat!");
                continue;
            }
        }

        let mut telemetrum_state: u8 = 0;

        if (time_now - iso_drogue_last_seen < 10000) && (iso_drogue_last_seen != 0) {
            telemetrum_state = telemetrum_state.wrapping_add(2u8.pow(7)); // bit 0
        }

        if (time_now - iso_main_last_seen < 10000) && (iso_main_last_seen != 0) {
            telemetrum_state = telemetrum_state.wrapping_add(2u8.pow(6)); // bit 1
        }

        let main_ok = time_now - main_last_seen < 2000;
        let drogue_ok = time_now - drogue_last_seen < 2000;

        let batt_read =
            BATT_READ_WATCH.receiver().expect("Could not get batt_read receiver").changed().await;

        let batt_ok = (batt_read > 99) as u8;
        let can_bus_ok = (main_ok && drogue_ok) as u8;
        let ers_status = (can_bus_ok != 0 && main_status && drogue_status) as u8;

        {
            let mut umb_on_unlocked = UMB_ON_MTX.lock().await;
            if let Some(umb_on_ref) = umb_on_unlocked.as_mut() {
                let shore_pow_status = umb_on_ref.is_low() as u8;

                let rocket_ready = (force_rocket_ready
                    || (shore_pow_status == 0 && batt_ok == 1 && ers_status == 1))
                    as u8;

                if rocket_ready == 1 {
                    rr_pin.set_high();
                    set_state(SenderStateField::RocketReady(true)).await;
                } else {
                    rr_pin.set_low();
                    set_state(SenderStateField::RocketReady(false)).await;
                }

                {
                    let mut buzz_mode_unlocked = BUZZER_MODE_MTX.lock().await;
                    if let Some(mode) = buzz_mode_unlocked.as_mut() {
                        match mode {
                            BuzzerMode::Off => {}
                            _ => {
                                if rocket_ready > 0 {
                                    *mode = BuzzerMode::High;
                                } else {
                                    *mode = BuzzerMode::Low;
                                }
                            }
                        }
                    }
                }

                let status_buf = [
                    telemetrum_state,
                    batt_read,
                    batt_ok,
                    shore_pow_status,
                    can_bus_ok,
                    ers_status,
                    rocket_ready,
                    0,
                ];

                let id = StandardId::new(TELEMETRUM_HEARTBEAT_ID).unwrap();
                let header = Header::new(Standard(id), 8, false);
                let frame = Frame::new(header, &status_buf).unwrap();

                let msg: CanTxChannelMsg = CanTxChannelMsg::new(false, frame);

                CAN_TX_CHANNEL.send(msg).await;

                if telemetrum_state != prev_telemetrum_state {
                    info!("Telemetrum state changed to {}", telemetrum_state);
                }

                if batt_ok != prev_batt_ok {
                    info!("Battery ok changed to {}", batt_ok);
                }

                if shore_pow_status != prev_shore_pow_status {
                    info!("Shore power status changed to {}", shore_pow_status);
                }

                if main_ok != prev_main_ok {
                    info!("Main ok changed to {}", main_ok);
                }

                if drogue_ok != prev_drogue_ok {
                    info!("Drogue ok changed to {}", drogue_ok);
                }

                if ers_status != prev_ers_status {
                    info!("Ers status changed to {}", ers_status);
                }

                if rocket_ready != prev_rocket_ready {
                    info!("Rocket ready changed to {}", rocket_ready);
                }

                prev_telemetrum_state = telemetrum_state;
                prev_batt_ok = batt_ok;
                prev_shore_pow_status = shore_pow_status;
                prev_main_ok = main_ok;
                prev_drogue_ok = drogue_ok;
                prev_ers_status = ers_status;
                prev_rocket_ready = rocket_ready;
            }
        }
        Timer::after_secs(1).await;
    }
}
