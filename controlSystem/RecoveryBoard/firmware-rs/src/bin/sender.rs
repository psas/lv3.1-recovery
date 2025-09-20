#![no_std]
#![no_main]

// TODO: Blinky command pin B14
// TODO: Command to print batt_read

use core::sync::atomic::{AtomicBool, Ordering};
use embedded_io_async::BufRead;
use portable_atomic::AtomicU64;

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler, SampleTime},
    bind_interrupts,
    can::{
        filter::Mask16,
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
        BufferedInterruptHandler, BufferedUart, BufferedUartRx, Config as UartConfig, DataBits,
        Parity, StopBits,
    },
};
use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex, channel::Channel, mutex::Mutex, pipe::Pipe,
    signal::Signal,
};
use embassy_time::{Instant, Timer};
use firmware_rs::shared::{
    adc::read_battery,
    buzzer::active_beep,
    can::can_writer,
    types::*,
    uart::{uart_writer, UART_READ_BUF_SIZE},
};
use static_cell::ConstStaticCell;
use {defmt_rtt as _, panic_probe as _};

const CAN_BITRATE: u32 = 1_000_000;
const DROGUE_ID: u16 = 0x100;
const MAIN_ID: u16 = 0x200;

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => InterruptHandler<ADC1>; });
bind_interrupts!(struct UsartIrqs { USART2 => BufferedInterruptHandler<USART2>; });

static ROCKET_READY: AtomicBool = AtomicBool::new(false);
static DROGUE_STATUS: AtomicBool = AtomicBool::new(false);
static MAIN_STATUS: AtomicBool = AtomicBool::new(false);
static DROGUE_LAST_SEEN: AtomicU64 = AtomicU64::new(0);
static MAIN_LAST_SEEN: AtomicU64 = AtomicU64::new(0);
static ISO_MAIN_TS: AtomicU64 = AtomicU64::new(0);
static ISO_DROGUE_TS: AtomicU64 = AtomicU64::new(0);

static TX_PIPE: Pipe<CriticalSectionRawMutex, UART_READ_BUF_SIZE> = Pipe::new();

static CAN_TX_CHANNEL: Channel<CriticalSectionRawMutex, CanTxChannelMsg, 10> = Channel::new();

static UMB_ON_MTX: UmbOnType = Mutex::new(None);

static BATT_READ_SIGNAL: BattOkSignalType = Signal::new();

static UART_TX_BUF_CELL: ConstStaticCell<[u8; UART_READ_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_READ_BUF_SIZE]);
static UART_RX_BUF_CELL: ConstStaticCell<[u8; UART_READ_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_READ_BUF_SIZE]);

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let umb_on = Input::new(p.PA8, Pull::Up);
    let iso_main = ExtiInput::new(p.PA6, p.EXTI6, Pull::Down);
    let iso_drogue = ExtiInput::new(p.PA5, p.EXTI5, Pull::Down);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium); // FIXME: use these?
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium); // FIXME: check speeds too
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

    // Set up CAN driver
    let mut can = Can::new(p.CAN, p.PA11, p.PA12, CanIrqs);
    can.modify_filters().enable_bank(
        0,
        Fifo::Fifo0,
        [
            Mask16::frames_with_std_id(unwrap!(StandardId::new(0x710)), StandardId::MAX),
            Mask16::frames_with_std_id(unwrap!(StandardId::new(0x720)), StandardId::MAX),
        ],
    );
    can.modify_config().set_bitrate(CAN_BITRATE).set_loopback(false).set_silent(false);
    // can.set_automatic_wakeup(true);

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
    let (uart_tx, uart_rx) = uart.split();

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(UMB_ON_MTX.lock().await) = Some(umb_on);
    }

    // unwrap!(spawner.spawn(active_beep(pwm, &ROCKET_READY)));
    unwrap!(spawner.spawn(uart_writer(uart_tx, &TX_PIPE)));
    unwrap!(spawner.spawn(uart_cli(uart_rx)));
    unwrap!(spawner.spawn(read_battery(adc, p.PB0, &BATT_READ_SIGNAL)));
    unwrap!(spawner.spawn(handle_iso_rising_edge(iso_drogue, &UMB_ON_MTX, DROGUE_ID)));
    unwrap!(spawner.spawn(handle_iso_rising_edge(iso_main, &UMB_ON_MTX, MAIN_ID)));
    unwrap!(spawner.spawn(telemetrum_heartbeat(&UMB_ON_MTX, rocket_ready_pin)));

    can.enable().await;
    let (can_tx, can_rx) = can.split();
    unwrap!(spawner.spawn(can_writer(can_tx, &CAN_TX_CHANNEL)));
    unwrap!(spawner.spawn(can_reader(can_rx, can)));

    // Keep main from returning. Needed for can_tx/can_rx or they get dropped
    core::future::pending::<()>().await;
}

async fn deploy(can_id: u16) {
    match can_id {
        DROGUE_ID => {
            info!("Releasing drogue");
        }
        MAIN_ID => {
            info!("Releasing main");
        }
        _ => {}
    }
    let id = unwrap!(StandardId::new(can_id));
    let header = Header::new(Id::Standard(id), 1, false);
    let frame = unwrap!(Frame::new(header, &[1; 0]));

    let msg = CanTxChannelMsg::new(true, frame);
    CAN_TX_CHANNEL.send(msg).await;
}

#[embassy_executor::task]
async fn uart_cli(mut rx: BufferedUartRx<'static>) {
    loop {
        let mut rbuf = [0u8; UART_READ_BUF_SIZE];
        let mut pos = 0usize;
        TX_PIPE.write_all(b"> ").await;
        loop {
            match rx.fill_buf().await {
                Ok(buf) => {
                    let n = buf.len();
                    rbuf[pos] = buf[0];
                    TX_PIPE.write_all(buf).await;
                    rx.consume(n);
                    if rbuf[pos] == 127 || rbuf[pos] == 27 {
                        // handle delete or backspace
                        rbuf[pos] = 0;
                        pos = pos.saturating_sub(1);
                        TX_PIPE.write_all(b"\x08 \x08").await;
                        continue;
                    }
                    if rbuf[pos] == b'\r' || rbuf[pos] == b'\n' {
                        TX_PIPE.write_all(b"\n").await;
                        break;
                    }
                    pos = pos.wrapping_add(1);
                }
                Err(e) => {
                    error!("{}", e);
                    break;
                }
            }
        }
        match &rbuf[..pos] {
            b"drogue" => {
                deploy(DROGUE_ID).await;
            }
            b"main" => {
                deploy(MAIN_ID).await;
            }
            b"help" => {
                TX_PIPE
                    .write_all(
                        b"\
                        Usage:\r\n\
                        drogue: trigger drogue ers\r\n\
                        main: trigger main ers\r\n\
                        help: display this message\r\n\
                        ",
                    )
                    .await;
            }
            _ => {
                TX_PIPE.write_all(b"Invalid command - please try again\r\n").await;
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
            Ok(envelope) => {
                let main_status_id = unwrap!(StandardId::new(0x720));
                let drogue_status_id = unwrap!(StandardId::new(0x710));

                match envelope.frame.id() {
                    Id::Standard(id) if *id == main_status_id => {
                        let status = envelope.frame.data()[0];
                        MAIN_LAST_SEEN.store(envelope.ts.as_millis(), Ordering::Relaxed);
                        MAIN_STATUS.store(status > 0, Ordering::Relaxed);
                        if status != prev_main_status {
                            info!("Main status changed to {}", status);
                        }
                        prev_main_status = status;
                    }
                    Id::Standard(id) if *id == drogue_status_id => {
                        let status = envelope.frame.data()[0];
                        DROGUE_LAST_SEEN.store(envelope.ts.as_millis(), Ordering::Relaxed);
                        DROGUE_STATUS.store(status > 0, Ordering::Relaxed);
                        if status != prev_drogue_status {
                            info!("Drogue status changed to {}", status);
                        }
                        prev_drogue_status = status;
                    }
                    _ => {}
                }
            }
            Err(e) => {
                error!("CAN Read Error: {}", e);
                can.sleep().await;
            }
        }
    }
}

#[embassy_executor::task(pool_size = 2)]
async fn handle_iso_rising_edge(
    mut iso: ExtiInput<'static>,
    umb_on: &'static UmbOnType,
    can_id: u16,
) -> () {
    loop {
        iso.wait_for_rising_edge().await;
        let time_now = Instant::now().as_millis();
        // telemetrum is signaling to deploy
        {
            let mut umb_on_unlocked = umb_on.lock().await;
            if let Some(umb_on_ref) = umb_on_unlocked.as_mut() {
                if umb_on_ref.is_high() {
                    deploy(can_id).await;
                }
            }
        }
        // record last deployment signal time and log to uart
        match can_id {
            DROGUE_ID => {
                ISO_DROGUE_TS.store(time_now, Ordering::Relaxed);
            }
            MAIN_ID => {
                ISO_MAIN_TS.store(time_now, Ordering::Relaxed);
            }
            _ => {}
        }
    }
}

#[embassy_executor::task]
async fn telemetrum_heartbeat(umb_on: &'static UmbOnType, mut rr_pin: Output<'static>) -> () {
    let mut prev_main_ok: bool = false;
    let mut prev_drogue_ok: bool = false;
    let mut prev_batt_ok: u8 = 0;
    let mut prev_ers_status: u8 = 0;
    let mut prev_telemetrum_state: u8 = 0;
    let mut prev_shore_pow_status: u8 = 0;
    let mut prev_rocket_ready: u8 = 0;

    loop {
        let time_now = Instant::now().as_millis();

        let mut telemetrum_state: u8 = 0;
        let iso_main_ts = ISO_MAIN_TS.load(Ordering::Relaxed);
        let iso_drogue_ts = ISO_DROGUE_TS.load(Ordering::Relaxed);

        if (time_now - iso_drogue_ts < 10000) && (iso_drogue_ts != 0) {
            telemetrum_state = telemetrum_state.wrapping_add(2u8.pow(7)); // bit 0
        }

        if (time_now - iso_main_ts < 10000) && (iso_main_ts != 0) {
            telemetrum_state = telemetrum_state.wrapping_add(2u8.pow(6)); // bit 1
        }

        let drogue_last_seen = DROGUE_LAST_SEEN.load(Ordering::Relaxed);
        let main_last_seen = MAIN_LAST_SEEN.load(Ordering::Relaxed);
        let main_ok = time_now - main_last_seen < 2000;
        let drogue_ok = time_now - drogue_last_seen < 2000;

        let batt_read = BATT_READ_SIGNAL.wait().await;
        let batt_ok = (batt_read > 99) as u8;
        let can_bus_ok = (main_ok && drogue_ok) as u8;
        let ers_status = (can_bus_ok != 0
            && MAIN_STATUS.load(Ordering::Relaxed)
            && DROGUE_STATUS.load(Ordering::Relaxed)) as u8;

        {
            let mut umb_on_unlocked = umb_on.lock().await;
            if let Some(umb_on_ref) = umb_on_unlocked.as_mut() {
                let shore_pow_status = umb_on_ref.is_low() as u8;

                let rocket_ready = (shore_pow_status == 0 && batt_ok == 1 && ers_status == 1) as u8;

                if rocket_ready == 1 {
                    rr_pin.set_high();
                    ROCKET_READY.store(true, Ordering::Relaxed);
                } else {
                    rr_pin.set_low();
                    ROCKET_READY.store(false, Ordering::Relaxed);
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

                let id = StandardId::new(0x700).unwrap();
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
