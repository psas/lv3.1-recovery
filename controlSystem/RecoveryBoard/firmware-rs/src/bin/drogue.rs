#![no_std]
#![no_main]

// TODO: Blinky command pin B14
// TODO: Command to print batt_read

use embassy_stm32::usart::BufferedInterruptHandler;
use embedded_io_async::BufRead;

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler, SampleTime},
    bind_interrupts,
    can::{
        filter::Mask16, Can, CanRx, Fifo, Rx0InterruptHandler, Rx1InterruptHandler,
        SceInterruptHandler, StandardId, TxInterruptHandler,
    },
    gpio::{Input, Level, Output, OutputType, Pull, Speed},
    peripherals::{ADC1, CAN, USART2},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{BufferedUart, BufferedUartRx, Config as UartConfig, DataBits, Parity, StopBits},
};
use embassy_sync::{mutex::Mutex, signal::Signal};
use firmware_rs::shared::{
    adc::read_battery,
    buzzer::active_beep,
    can::{can_writer, CAN_BITRATE, CAN_TX_CHANNEL},
    types::*,
    uart::{uart_writer, TX_PIPE, UART_READ_BUF_SIZE, UART_RX_BUF_CELL, UART_TX_BUF_CELL},
};
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

static BATT_READ_SIGNAL: BattOkSignalType = Signal::new();

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let umb_on = Input::new(p.PA8, Pull::Up);
    let _can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let _can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);

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

    unwrap!(spawner.spawn(active_beep(pwm, None)));
    unwrap!(spawner.spawn(uart_writer(uart_tx, &TX_PIPE)));
    unwrap!(spawner.spawn(uart_cli(uart_rx)));
    unwrap!(spawner.spawn(read_battery(adc, p.PB0, &BATT_READ_SIGNAL)));

    // enable at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    unwrap!(spawner.spawn(can_writer(can_tx, &CAN_TX_CHANNEL)));
    unwrap!(spawner.spawn(can_reader(can_rx, can)));

    // Keep main from returning. Needed for can_tx/can_rx or they get dropped
    core::future::pending::<()>().await;
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
            b"help" => {
                TX_PIPE
                    .write_all(
                        b"\
                        Usage:\r\n\
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
    loop {
        match can_rx.read().await {
            Ok(envelope) => match envelope.frame.id() {
                _ => {}
            },
            Err(e) => {
                error!("CAN Read Error: {}", e);
                can.sleep().await;
            }
        }
    }
}
