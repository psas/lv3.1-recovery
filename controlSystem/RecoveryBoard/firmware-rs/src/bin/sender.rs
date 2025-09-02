#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler},
    bind_interrupts,
    can::{
        filter::Mask32, Can, Fifo, Rx0InterruptHandler, Rx1InterruptHandler, SceInterruptHandler,
        TxInterruptHandler,
    },
    dac::{self, Dac},
    gpio::OutputType,
    peripherals::{ADC1, CAN, USART3},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{
        Config as UartConfig, DataBits, InterruptHandler as UartInterruptHandler, Parity, StopBits,
        Uart,
    },
};
use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex, pipe::Pipe, signal::Signal,
};
use firmware_rs::shared::{
    adc::read_battery,
    buzzer::{active_beep, play_power_on},
    can::echo_can,
    types::*,
    uart::{uart_reader, uart_writer, READ_BUF_SIZE},
};
use {defmt_rtt as _, panic_probe as _};

const CAN_BITRATE: u32 = 1_000_000; // 1Mbps

bind_interrupts!(struct CanIrqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});
bind_interrupts!(struct AdcIrqs { ADC1_COMP => InterruptHandler<ADC1>; });

// TODO: Switch to USART2 before deployment.
bind_interrupts!(struct UsartIrqs { USART3_8 => UartInterruptHandler<USART3>; });

static TX_PIPE: Pipe<CriticalSectionRawMutex, READ_BUF_SIZE> = Pipe::new();

static CAN_MTX: CanType = Mutex::new(None);
static PWM_MTX: PwmType = Mutex::new(None);
static ADC_MTX: AdcType = Mutex::new(None);

static BATT_OK_SIGNAL: BattOkSignalType = Signal::new();

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    info!("Initialized stm32");

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
    can.modify_config().set_bitrate(CAN_BITRATE);
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());
    can.enable().await;

    // Set up ADC driver
    let adc = Adc::new(p.ADC1, AdcIrqs);

    // Set up DAC driver
    let mut dac = Dac::new(p.DAC1, p.DMA1_CH3, p.DMA1_CH4, p.PA4, p.PA5);

    // Set up UART driver
    let mut uart_config = UartConfig::default();
    uart_config.baudrate = 115200;
    uart_config.parity = Parity::ParityNone;
    uart_config.data_bits = DataBits::DataBits8;
    uart_config.stop_bits = StopBits::STOP1;

    // TODO: Switch to PA2/PA3 before deployment.
    let uart = Uart::new(p.USART3, p.PC11, p.PC10, UsartIrqs, p.DMA1_CH2, p.DMA1_CH5, uart_config)
        .expect("Uart Config Error");

    // split uart so that we can wait for messages coming in while still transmitting messages
    let (uart_tx, uart_rx) = uart.split();

    {
        // Put drivers into mutex. inner scope so that mutex is unlocked when out of scope
        *(PWM_MTX.lock().await) = Some(pwm);
        *(CAN_MTX.lock().await) = Some(can);
        *(ADC_MTX.lock().await) = Some(adc);
    }

    play_power_on(&PWM_MTX).await;

    unwrap!(spawner.spawn(active_beep(&PWM_MTX)));
    unwrap!(spawner.spawn(echo_can(&CAN_MTX)));
    unwrap!(spawner.spawn(uart_reader(uart_rx, &TX_PIPE)));
    unwrap!(spawner.spawn(uart_writer(uart_tx, &TX_PIPE)));
    unwrap!(spawner.spawn(read_battery(&BATT_OK_SIGNAL, &ADC_MTX, p.PB0)));

    dac.set(dac::DualValue::Bit12Right(2048, 2048));

    // batt_ok signal:
    // let sig: bool = BATT_OK_SIGNAL.wait().await;
    // info!("Batt ok: {}", sig); // Batt ok: true/false
}
