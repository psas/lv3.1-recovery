#![no_std]
#![no_main]

use core::sync::atomic::Ordering::Relaxed;

use defmt::{error, info, panic, unwrap};
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler},
    bind_interrupts,
    can::{
        filter::Mask32, Can, Fifo, Rx0InterruptHandler, Rx1InterruptHandler, RxBuf,
        SceInterruptHandler, TxBuf, TxInterruptHandler,
    },
    exti::{ExtiInput, InterruptHandler as ExtiInterruptHandler},
    gpio::{Input, Level, Output, OutputType, Pull, Speed},
    mode::Async,
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
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex, signal::Signal};
use embassy_time::{Duration, Instant, Timer};
use firmware_rs::{
    adc::{read_battery, BATT_READ_WATCH},
    blink::blink_led,
    buzzer::{active_beep, BuzzerMode, BUZZER_MODE_MTX},
    can::{
        can_writer, CAN_BITRATE, CAN_BUF_SIZE, CAN_MTX, CAN_RX_BUF, CAN_TX_BUF, DROGUE_DEPLOY_ID,
        MAIN_DEPLOY_ID,
    },
    sender::{
        can::{
            can_reader, send_deploy_msg, send_heartbeat, HeartbeatContext, CAN_SIGNAL,
            DROGUE_ACKNOWLEDGE, MAIN_ACKNOWLEDGE,
        },
        cli::{self, serial_read_task, serial_write_task},
        cmd::{async_cmd_handler, SenderCmd, ASYNC_CMD_CHANNEL},
        state::SenderState,
    },
    types::ShorePowOnType,
    uart::{UART_RX_BUF_CELL, UART_TX_BUF_CELL},
    wdg::i_wdg,
};
use ufmt::uwrite;
use {defmt_rtt as _, panic_probe as _};

const MAIN_LOOP_INTERVAL_MS: u64 = 10;
const HEARTBEAT_INTERVAL_MS: u64 = 1000;

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => InterruptHandler<ADC1>; });
bind_interrupts!(struct UsartIrqs { USART2 => BufferedInterruptHandler<USART2>; });
bind_interrupts!(struct ExtiIrqs {
    EXTI4_15 =>
    ExtiInterruptHandler<embassy_stm32::interrupt::typelevel::EXTI4_15>;
});

static SHORE_POW_ON_MTX: ShorePowOnType = Mutex::new(None);

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let shore_pow_on_pin = Input::new(p.PA8, Pull::Up);
    let iso_main = ExtiInput::new(p.PA6, p.EXTI6, Pull::Down, ExtiIrqs);
    let iso_drogue = ExtiInput::new(p.PA5, p.EXTI5, Pull::Down, ExtiIrqs);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);
    let mut rocket_ready_pin = Output::new(p.PA7, Level::Low, Speed::Medium);

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

    let mut can = Can::new(p.CAN, p.PA11, p.PA12, CanIrqs);
    can.modify_config().set_bitrate(CAN_BITRATE).set_loopback(false).set_silent(false);
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());

    let mut adc = Adc::new(p.ADC1, AdcIrqs);
    adc.set_resolution(embassy_stm32::adc::Resolution::BITS12);

    let mut uart_config = UartConfig::default();
    uart_config.baudrate = 115200;
    uart_config.parity = Parity::ParityNone;
    uart_config.data_bits = DataBits::DataBits8;
    uart_config.stop_bits = StopBits::STOP1;

    let uart = unwrap!(
        BufferedUart::new(
            p.USART2,
            p.PA3,
            p.PA2,
            UART_TX_BUF_CELL.take(),
            UART_RX_BUF_CELL.take(),
            UsartIrqs,
            uart_config,
        ),
        "Uart Config Error"
    );
    let (uart_tx, uart_rx) = uart.split();

    let (uart_cli, serial_write_ctx, serial_read_ctx) =
        match cli::init(uart_tx, uart_rx, "sender@ers> ") {
            Ok(ctx) => ctx,
            Err(_) => {
                error!("Failed to init CLI");
                panic!()
            }
        };

    spawner.spawn(unwrap!(blink_led(p.PB14)));
    spawner.spawn(unwrap!(active_beep(pwm)));
    spawner.spawn(unwrap!(read_battery(adc, p.PB0)));
    spawner.spawn(unwrap!(serial_write_task(serial_write_ctx)));
    spawner.spawn(unwrap!(serial_read_task(serial_read_ctx)));
    spawner.spawn(unwrap!(i_wdg(p.IWDG)));

    let mut batt_rcvr =
        unwrap!(BATT_READ_WATCH.receiver(), "failed to create batt_read_watch receiver");

    // enable CAN at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    let can_txb = can_tx.buffered(CAN_TX_BUF.init(TxBuf::<CAN_BUF_SIZE>::new()));
    let can_rxb = can_rx.buffered(CAN_RX_BUF.init(RxBuf::<CAN_BUF_SIZE>::new()));

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(SHORE_POW_ON_MTX.lock().await) = Some(shore_pow_on_pin);
        *(CAN_MTX.lock().await) = Some(can);
        *(BUZZER_MODE_MTX.lock().await) = Some(BuzzerMode::Off);
    }

    spawner.spawn(unwrap!(can_writer(can_txb)));
    spawner.spawn(unwrap!(can_reader(can_rxb)));
    spawner.spawn(unwrap!(handle_iso_rising_edge(iso_drogue, DROGUE_DEPLOY_ID)));
    spawner.spawn(unwrap!(handle_iso_rising_edge(iso_main, MAIN_DEPLOY_ID)));
    spawner.spawn(unwrap!(async_cmd_handler()));

    let mut state: SenderState = SenderState::default();

    // Set up main control loop
    let mut next_iter_start = Instant::now();
    let mut last_heartbeat_time = 0u64;

    loop {
        next_iter_start += Duration::from_millis(MAIN_LOOP_INTERVAL_MS);
        Timer::at(next_iter_start).await;
        let time_now = Instant::now().as_millis();

        // ==================================================================================
        // update state
        {
            let mut shore_pow_on_unlocked = SHORE_POW_ON_MTX.lock().await;
            if let Some(spo_ref) = shore_pow_on_unlocked.as_mut() {
                let shore_pow_on = spo_ref.is_low();
                state.shore_pow_on = shore_pow_on
            } else {
                error!("shore power mutex not initialized");
            };
        }

        let batt_read = batt_rcvr.get().await;

        if CAN_SIGNAL.signaled() {
            let sig = unwrap!(CAN_SIGNAL.try_take(), "CAN signaled but main failed to read");
            state.drogue_last_seen = sig.drogue_last_seen;
            state.main_last_seen = sig.main_last_seen;
            state.drogue_ready = sig.drogue_ready;
            state.main_ready = sig.main_ready;
        }

        if ISO_DROGUE_TS_SIGNAL.signaled() {
            let sig = unwrap!(
                ISO_DROGUE_TS_SIGNAL.try_take(),
                "iso_drogue signaled but main failed to read"
            );
            state.iso_drogue_last_seen = sig;
        }

        if ISO_MAIN_TS_SIGNAL.signaled() {
            let sig =
                unwrap!(ISO_MAIN_TS_SIGNAL.try_take(), "iso_main signaled but main failed to read");
            state.iso_main_last_seen = sig;
        }

        let drogue_can_ok = time_now - state.drogue_last_seen < 2000;
        let main_can_ok = time_now - state.main_last_seen < 2000;
        let can_bus_ok = drogue_can_ok && main_can_ok;
        let ers_ready = can_bus_ok && state.drogue_ready && state.main_ready;

        let rocket_ready =
            state.force_rocket_ready || (state.shore_pow_on && (batt_read > 99) && ers_ready);

        if rocket_ready {
            rocket_ready_pin.set_high();
        }

        state.rocket_ready = rocket_ready;

        // ==================================================================================
        // handle pending commands
        let async_cmd_sender = ASYNC_CMD_CHANNEL.sender();
        let _ = uart_cli.process_pending_commands(|cli, command| {
            match command {
                SenderCmd::State => {
                    for field in state.iter() {
                        if let Err(e) = uwrite!(cli.writer(), "{}", field) {
                            error!("failed to write from cli: {}", e);
                        }
                    }
                }
                SenderCmd::Drogue => {
                    if let Err(e) = async_cmd_sender.try_send(SenderCmd::Drogue) {
                        error!("failed to send async cmd: {}", e);
                    }
                }
                SenderCmd::Main => {
                    if let Err(e) = async_cmd_sender.try_send(SenderCmd::Main) {
                        error!("failed to send async cmd: {}", e);
                    }
                }
                SenderCmd::Rr => state.force_rocket_ready = !state.force_rocket_ready,
                SenderCmd::Batt => {
                    if let Err(e) = uwrite!(cli.writer(), "{}", batt_read) {
                        error!("failed to write from uart: {}", e);
                    }
                }
                SenderCmd::Beep => {
                    if let Err(e) = async_cmd_sender.try_send(SenderCmd::Beep) {
                        error!("failed to send async cmd: {}", e);
                    }
                }
                SenderCmd::Version => {
                    if let Err(e) = uwrite!(cli.writer(), "{}", env!("CARGO_PKG_VERSION")) {
                        error!("failed to write from uart: {}", e);
                    }
                }
            }
            Ok(())
        });

        // ==================================================================================
        // update buzzer mode
        {
            let mut buzz_mode_unlocked = BUZZER_MODE_MTX.lock().await;
            if let Some(mode) = buzz_mode_unlocked.as_mut() {
                match mode {
                    BuzzerMode::Off => {}
                    _ => {
                        if state.rocket_ready {
                            *mode = BuzzerMode::High;
                        } else {
                            *mode = BuzzerMode::Low;
                        }
                    }
                }
            }
        }

        // ==================================================================================
        // send heartbeat message
        if time_now - last_heartbeat_time > HEARTBEAT_INTERVAL_MS {
            // create heartbeat context
            let mut sender_state: u8 = 0;

            if (time_now - state.iso_drogue_last_seen < 10000) && (state.iso_drogue_last_seen != 0)
            {
                sender_state = sender_state.wrapping_add(2u8.pow(7)); // bit 0
            }

            if (time_now - state.iso_main_last_seen < 10000) && (state.iso_main_last_seen != 0) {
                sender_state = sender_state.wrapping_add(2u8.pow(6)); // bit 1
            }

            let ctx = HeartbeatContext::new(
                sender_state,
                batt_read,
                state.shore_pow_on,
                can_bus_ok,
                state.drogue_ready,
                state.main_ready,
                state.rocket_ready,
            );

            if send_heartbeat(ctx).await.is_err() {
                panic!()
            }

            last_heartbeat_time = time_now;
        }
    }
}

static ISO_DROGUE_TS_SIGNAL: Signal<CriticalSectionRawMutex, u64> = Signal::new();
static ISO_MAIN_TS_SIGNAL: Signal<CriticalSectionRawMutex, u64> = Signal::new();

#[embassy_executor::task(pool_size = 2)]
async fn handle_iso_rising_edge(mut iso: ExtiInput<'static, Async>, can_id: u16) -> () {
    // waits for a rising edge on the specified gpio and responds by firing
    // a can msg with the specified id as long as shore power is off.
    loop {
        info!("awaiting signal from telemetrum");
        iso.wait_for_rising_edge().await;
        info!("telemetrum signalling to deploy: {}", can_id);
        let time_now = Instant::now().as_millis();
        if can_id == DROGUE_DEPLOY_ID {
            info!("setting drogue acknowledge to false");
            DROGUE_ACKNOWLEDGE.store(false, Relaxed);
        } else {
            info!("setting main acknowledge to false");
            MAIN_ACKNOWLEDGE.store(false, Relaxed);
        }
        {
            let mut shore_power_on_unlocked = SHORE_POW_ON_MTX.lock().await;
            if let Some(spo_ref) = shore_power_on_unlocked.as_mut() {
                if spo_ref.is_high() {
                    info!("sending deploy message");
                    if send_deploy_msg(can_id).await.is_err() {
                        panic!()
                    }
                }
            }
        }
        // record last deployment signal time
        match can_id {
            DROGUE_DEPLOY_ID => {
                ISO_DROGUE_TS_SIGNAL.signal(time_now);
            }
            MAIN_DEPLOY_ID => {
                ISO_MAIN_TS_SIGNAL.signal(time_now);
            }
            _ => {}
        }
    }
}
