/* Parachute boards main entrypoint */

#![no_std]
#![no_main]

use core::str::FromStr;
use defmt::{error, info, panic, unwrap};
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler as AdcInterruptHandler},
    bind_interrupts,
    can::{
        filter::Mask32, Can, Fifo, Rx0InterruptHandler, Rx1InterruptHandler, RxBuf,
        SceInterruptHandler, TxBuf, TxInterruptHandler,
    },
    dac::Dac,
    dma::InterruptHandler as DmaInterruptHandler,
    flash::Flash,
    gpio::{Input, Level, Output, OutputType, Pull, Speed},
    peripherals::{ADC1, CAN, DMA1_CH3, DMA1_CH4, USART2},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{
        BufferedInterruptHandler, BufferedUart, Config as UartConfig, DataBits, Parity, StopBits,
    },
};
use embassy_time::{Duration, Instant, Timer};
use firmware_rs::{
    adc::{read_battery_from_ref, ADC_MTX, BATT_READ_WATCH},
    blink::blink_led,
    buzzer::{active_beep, BuzzerMode, BUZZER_MODE_MTX},
    can::{can_writer, CAN_BITRATE, CAN_BUF_SIZE, CAN_MTX, CAN_RX_BUF, CAN_TX_BUF},
    flash::{
        FLASH_MTX, MOTOR_ACT_SECTOR_OFFSET, MOTOR_ACT_SECTOR_SIZE, SENSOR_LIMIT_SECTOR_OFFSET,
        SENSOR_LIMIT_SECTOR_SIZE,
    },
    parachute::{
        can::{can_reader, send_heartbeat, HeartbeatCtx, CAN_SIGNAL},
        cli::{self, serial_read_task, serial_write_task},
        cmd::{async_cmd_handler, AsyncCmd, ChuteCmd, ASYNC_CMD_CHANNEL},
        motor::{Motor, MOTOR_MTX},
        ring::{read_pos_sensor, Ring, RingPosition, SensorLimits, RING_MTX, RING_POSITION_WATCH},
        state::ChuteState,
    },
    uart::{UART_RX_BUF_CELL, UART_TX_BUF_CELL},
    wdg::i_wdg,
};
use ufmt::uwrite;
use {defmt_rtt as _, panic_probe as _};

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => AdcInterruptHandler<ADC1>; });
bind_interrupts!(struct DacIrqs {
    DMA1_CH2_3_DMA2_CH1_2 => DmaInterruptHandler<DMA1_CH3>;
    DMA1_CH4_7_DMA2_CH3_5 => DmaInterruptHandler<DMA1_CH4>;
});
bind_interrupts!(struct UsartIrqs { USART2 => BufferedInterruptHandler<USART2>; });

const MAIN_LOOP_INTERVAL_MS: u64 = 10;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let shore_pow_on_pin = Input::new(p.PA8, Pull::Up);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);

    let flash = Flash::new_blocking(p.FLASH);

    {
        // Init the flash mutex
        // Ring depends on it being available to construct
        *(FLASH_MTX.lock().await) = Some(flash);
    }

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
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());
    can.modify_config().set_bitrate(CAN_BITRATE).set_loopback(false).set_silent(false);

    // Set up ADC driver
    let mut adc = Adc::new(p.ADC1, AdcIrqs);
    adc.set_resolution(embassy_stm32::adc::Resolution::BITS12);

    // Set up UART driver
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

    let dac = Dac::new(p.DAC1, p.DMA1_CH3, p.DMA1_CH4, DacIrqs, p.PA4, p.PA5);

    let motor = Motor::new(p.PB4, p.PB5, p.PB6, p.PB7, dac);
    let ring = Ring::new(p.PA0, p.PA1, p.PB1).await;

    #[allow(unused)] // It is immediately used. RA doesn't know because of cfg directives
    let mut prompt = "BOARD_ENV_ERR";

    #[cfg(drogue)]
    {
        prompt = "drogue@ers> "
    }

    #[cfg(main)]
    {
        prompt = "main@ers> "
    }

    let (chute_cli, serial_write_ctx, serial_read_ctx) = match cli::init(uart_tx, uart_rx, prompt) {
        Ok(ctx) => ctx,
        Err(e) => {
            error!("Failed to create CLI: {}", e);
            panic!()
        }
    };

    #[allow(unused_mut, unused_assignments)] // suppress warning when not compiling with disable_beep cfg
    let mut buzz_mode = BuzzerMode::Low;

    #[cfg(disable_beep)]
    {
        buzz_mode = BuzzerMode::Off;
    }

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(ADC_MTX.lock().await) = Some(adc);
        *(RING_MTX.lock().await) = Some(ring);
        *(MOTOR_MTX.lock().await) = Some(motor);
        *(BUZZER_MODE_MTX.lock().await) = Some(buzz_mode);
    }

    spawner.spawn(unwrap!(blink_led(p.PB14)));
    spawner.spawn(unwrap!(active_beep(pwm)));
    spawner.spawn(unwrap!(read_battery_from_ref(p.PB0)));
    spawner.spawn(unwrap!(read_pos_sensor()));
    spawner.spawn(unwrap!(serial_write_task(serial_write_ctx)));
    spawner.spawn(unwrap!(serial_read_task(serial_read_ctx)));
    spawner.spawn(unwrap!(async_cmd_handler()));
    spawner.spawn(unwrap!(i_wdg(p.IWDG)));

    // enable can at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    let can_txb = can_tx.buffered(CAN_TX_BUF.init(TxBuf::<CAN_BUF_SIZE>::new()));
    let can_rxb = can_rx.buffered(CAN_RX_BUF.init(RxBuf::<CAN_BUF_SIZE>::new()));

    {
        *(CAN_MTX.lock().await) = Some(can);
    }

    spawner.spawn(unwrap!(can_writer(can_txb)));
    spawner.spawn(unwrap!(can_reader(can_rxb)));

    // Set up main control loop
    let mut next_iter_start = Instant::now();
    let mut last_heartbeat_time = 0u64;
    let mut ring_pos_rcvr =
        unwrap!(RING_POSITION_WATCH.receiver(), "unable to get ring position receiver");

    let mut batt_read_rcvr =
        unwrap!(BATT_READ_WATCH.receiver(), "unable to get batt read receiver");

    let mut state = ChuteState::default();

    loop {
        next_iter_start += Duration::from_millis(MAIN_LOOP_INTERVAL_MS);
        Timer::at(next_iter_start).await;
        let time_now = Instant::now().as_millis();

        // =========================================================================
        // update state
        let shore_power_on = shore_pow_on_pin.is_low();
        state.shore_power_on = shore_power_on;

        if CAN_SIGNAL.signaled() {
            let sig = unwrap!(CAN_SIGNAL.try_take(), "CAN signaled but main was unable to read it");
            state.sender_last_seen = sig;
        }

        let batt_read = batt_read_rcvr.get().await;
        let batt_ok = batt_read > 99;
        let sender_can_ok = time_now - state.sender_last_seen < 2000;
        let ring_pos = ring_pos_rcvr.get().await;

        state.ready =
            ring_pos == RingPosition::Locked && !shore_power_on && batt_ok && sender_can_ok;

        let async_cmd_sender = ASYNC_CMD_CHANNEL.sender();

        // ==================================================================================
        // update buzzer mode
        {
            let mut buzz_mode_unlocked = BUZZER_MODE_MTX.lock().await;
            if let Some(mode) = buzz_mode_unlocked.as_mut() {
                match mode {
                    BuzzerMode::Off => {}
                    _ => {
                        if state.ready {
                            *mode = BuzzerMode::High;
                        } else {
                            *mode = BuzzerMode::Low;
                        }
                    }
                }
            }
        }

        // =========================================================================
        // process pending commands
        let _ = chute_cli.process_pending_commands(|cli, command| {
            // NOTE: Any command that writes a response back to uart must be sync as
            // we don't have access to the cli writer anywhere other than here.
            // Prefetching can be used to get around this limitation e.g. the batt or state cmds.
            match command {
                ChuteCmd::State => {
                    for field in state.iter() {
                        let _ = uwrite!(cli.writer(), "{}", field);
                    }
                }
                ChuteCmd::Batt => {
                    let _ = uwrite!(cli.writer(), "{}", batt_read);
                }
                ChuteCmd::Beep => {
                    if async_cmd_sender.try_send(AsyncCmd::Beep).is_err() {
                        error!("Could not send beep command to async handler");
                    }
                }
                ChuteCmd::L { force, pulse } => {
                    if async_cmd_sender.try_send(AsyncCmd::L { force, pulse }).is_err() {
                        error!("unable to send lock command to async handler")
                    }
                }
                ChuteCmd::U { force, pulse } => {
                    if !state.shore_power_on {
                        if async_cmd_sender.try_send(AsyncCmd::U { force, pulse }).is_err() {
                            error!("unable to send unlock commmand to async handler");
                        };
                    } else {
                        let _ =
                            uwrite!(cli.writer(), "shore power is on - turn off to unlock ring");
                    }
                }
                ChuteCmd::Pos => {
                    let _ = uwrite!(cli.writer(), "{}", ring_pos);
                }
                ChuteCmd::Acts => {
                    let mut bytes = [0u8; (MOTOR_ACT_SECTOR_SIZE / 8) as usize];

                    // Nothing should be accessing the flash except at startup or in another
                    // command, so try_lock should be safe here
                    if let Ok(mut flash_unlocked) = FLASH_MTX.try_lock() {
                        if let Some(flash) = flash_unlocked.as_mut() {
                            if let Err(e) = flash.blocking_read(MOTOR_ACT_SECTOR_OFFSET, &mut bytes)
                            {
                                error!("error reading flash: {}", e);
                            }
                        }
                    } else {
                        error!("failed to unlock flash mutex");
                    }

                    let _ = uwrite!(cli.writer(), "{}", bytes[0] + 1);
                }
                ChuteCmd::Erase => {
                    if async_cmd_sender.try_send(AsyncCmd::Erase).is_err() {
                        error!("unable to send erase command to async handler");
                    }
                }
                ChuteCmd::Limits { print, set } => {
                    if print {
                        let mut buf = [0u8; (SENSOR_LIMIT_SECTOR_SIZE / 8) as usize];

                        // Nothing should be accessing the flash except at startup or in another
                        // command, so try_lock should be safe here
                        if let Ok(mut flash_unlocked) = FLASH_MTX.try_lock() {
                            if let Some(flash) = flash_unlocked.as_mut() {
                                if let Err(e) =
                                    flash.blocking_read(SENSOR_LIMIT_SECTOR_OFFSET, &mut buf)
                                {
                                    let _ = uwrite!(cli.writer(), "unable to read flash");
                                    error!("error reading flash: {}", e);
                                }
                            }
                        } else {
                            let _ = uwrite!(cli.writer(), "unable to access flash mutex");
                            error!("failed to unlock flash mutex");
                        }

                        fn two_u8_to_u16(b1: u8, b0: u8) -> u16 {
                            // combine two u8s into one u16
                            ((b1 as u16) << 8) + b0 as u16
                        }

                        let sensor1_limits = SensorLimits::new(
                            two_u8_to_u16(buf[0], buf[1]),
                            two_u8_to_u16(buf[2], buf[3]),
                            two_u8_to_u16(buf[4], buf[5]),
                            two_u8_to_u16(buf[6], buf[7]),
                        );
                        let sensor2_limits = SensorLimits::new(
                            two_u8_to_u16(buf[8], buf[9]),
                            two_u8_to_u16(buf[10], buf[11]),
                            two_u8_to_u16(buf[12], buf[13]),
                            two_u8_to_u16(buf[14], buf[15]),
                        );

                        let _ = uwrite!(
                            cli.writer(),
                            "Sensor 1:\r\n{}Sensor 2:\r\n{}",
                            sensor1_limits,
                            sensor2_limits
                        );
                    } else {
                        info!("{}", set);
                        if let Some(lims) = set {
                            info!("lims: {}", lims);
                            let limits: heapless::Vec<heapless::String<32>, 10> = lims
                                .split(',')
                                .map(|s| match heapless::String::from_str(s) {
                                    Ok(sh) => sh,
                                    Err(_) => {
                                        error!("unable to create String from {}", s);
                                        panic!()
                                    }
                                })
                                .collect();

                            if limits.len() != 8 {
                                let _ = uwrite!(
                                    cli.writer(),
                                    "Error: Expected 8 values but received {}",
                                    limits.len()
                                );
                            } else {
                                for lim in &limits {
                                    info!("{}", lim.as_str());
                                }

                                if async_cmd_sender
                                    .try_send(AsyncCmd::SetLimits { limits })
                                    .is_err()
                                {
                                    error!("could not send limits command to async handler");
                                }
                            }
                        } else {
                            let _ = uwrite!(cli.writer(), "no limits provided");
                        }
                    }
                }
                ChuteCmd::Version => {
                    if let Err(e) = uwrite!(cli.writer(), "{}", env!("CARGO_PKG_VERSION")) {
                        error!("failed to write from uart: {}", e);
                    }
                }
            }
            Ok(())
        });

        // =========================================================================
        // heartbeat
        if time_now - last_heartbeat_time > 1000 {
            let sender_ok = (time_now - state.sender_last_seen) < 2000;

            let ready = ring_pos == RingPosition::Locked
                && shore_pow_on_pin.is_high()
                && batt_ok
                && sender_ok;

            let heartbeat_ctx = HeartbeatCtx::new(
                ring_pos,
                batt_read,
                shore_pow_on_pin.is_high(),
                sender_ok,
                ready,
            );

            if (send_heartbeat(heartbeat_ctx).await).is_err() {
                panic!()
            };
            last_heartbeat_time = time_now;
        }
    }
}
