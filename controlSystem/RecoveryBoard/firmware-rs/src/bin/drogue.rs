#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::{
    adc::{Adc, InterruptHandler, SampleTime},
    bind_interrupts,
    can::{
        Can, CanRx, Fifo, Rx0InterruptHandler, Rx1InterruptHandler, SceInterruptHandler,
        TxInterruptHandler,
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
use embassy_sync::{mutex::Mutex, signal::Signal};
use embedded_io_async::Write;
use firmware_rs::shared::{
    adc::read_battery_from_ref,
    can::{can_writer, CAN_BITRATE, CAN_TX_CHANNEL},
    types::*,
    uart::{IO, UART_BUF_SIZE, UART_RX_BUF_CELL, UART_TX_BUF_CELL},
};
use noline::builder::EditorBuilder;
use {defmt_rtt as _, panic_probe as _};

#[derive(Default)]
pub struct DrogueState {
    pub rocket_ready: bool,
    pub force_rocket_ready: bool,
    pub drogue_status: bool,
    pub main_status: bool,
    pub drogue_last_seen: u64,
    pub main_last_seen: u64,
    pub iso_main_last_seen: u64,
    pub iso_drogue_last_seen: u64,
}

#[derive(Debug)]
pub enum DrogueStateField {
    RocketReady(bool),
    ForceRocketReady(bool),
    DrogueStatus(bool),
    MainStatus(bool),
    DrogueLastSeen(u64),
    MainLastSeen(u64),
    IsoMainLastSeen(u64),
    IsoDrogueLastSeen(u64),
}

pub struct DrogueStateIter<'a> {
    state_fields: &'a DrogueState,
    index: usize,
}

impl DrogueState {
    pub fn iter(&self) -> DrogueStateIter<'_> {
        DrogueStateIter { state_fields: self, index: 0 }
    }
}

impl<'a> Iterator for DrogueStateIter<'a> {
    type Item = DrogueStateField;

    fn next(&mut self) -> Option<Self::Item> {
        let result = match self.index {
            0 => Some(DrogueStateField::RocketReady(self.state_fields.rocket_ready)),
            1 => Some(DrogueStateField::ForceRocketReady(self.state_fields.force_rocket_ready)),
            2 => Some(DrogueStateField::DrogueStatus(self.state_fields.drogue_status)),
            3 => Some(DrogueStateField::MainStatus(self.state_fields.main_status)),
            4 => Some(DrogueStateField::DrogueLastSeen(self.state_fields.drogue_last_seen)),
            5 => Some(DrogueStateField::MainLastSeen(self.state_fields.main_last_seen)),
            6 => Some(DrogueStateField::IsoMainLastSeen(self.state_fields.iso_main_last_seen)),
            7 => Some(DrogueStateField::IsoDrogueLastSeen(self.state_fields.iso_drogue_last_seen)),
            _ => None,
        };

        if result.is_some() {
            self.index += 1;
        }

        result
    }
}

impl core::fmt::Display for DrogueStateField {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            &Self::RocketReady(val) => {
                core::write!(f, "Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            &Self::ForceRocketReady(val) => {
                core::write!(f, "Force Rocket Ready: {}", if val { "YES" } else { "NO" })
            }
            &Self::DrogueStatus(val) => {
                core::write!(f, "Drogue Status: {}", if val { "OK" } else { "ERR" })
            }
            &Self::MainStatus(val) => {
                core::write!(f, "Main Status: {}", if val { "OK" } else { "ERR" })
            }
            Self::DrogueLastSeen(val) => core::write!(f, "Drogue last seen: {}ms", val),
            Self::MainLastSeen(val) => core::write!(f, "Main last seen: {}ms", val),
            Self::IsoDrogueLastSeen(val) => core::write!(f, "Iso drogue last seen: {}ms", val),
            Self::IsoMainLastSeen(val) => core::write!(f, "Iso main last seen: {}ms", val),
        }
    }
}

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
static ADC_MTX: AdcType = Mutex::new(None);

static BATT_READ_SIGNAL: BattOkSignalType = Signal::new();

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let _deploy_1 = Output::new(p.PB4, Level::Low, Speed::Medium);
    let _deploy_2 = Output::new(p.PB5, Level::Low, Speed::Medium);
    let _motor_fail = Input::new(p.PB7, Pull::Up);
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

    // Set up CAN driver
    let mut can = Can::new(p.CAN, p.PA11, p.PA12, CanIrqs);
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());
    can.modify_config().set_bitrate(CAN_BITRATE).set_loopback(false).set_silent(false);

    // Set up ADC driver
    let mut adc = Adc::new(p.ADC1, AdcIrqs);
    adc.set_sample_time(SampleTime::CYCLES239_5);
    adc.set_resolution(embassy_stm32::adc::Resolution::BITS12);

    // Set up UART driver'
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

    let mut _dac = Dac::new(p.DAC1, p.DMA1_CH3, p.DMA1_CH4, p.PA4, p.PA5);

    {
        // Put peripherals into mutex if shared among tasks.
        // Inner scope so that mutex is unlocked when out of scope
        *(ADC_MTX.lock().await) = Some(adc);
        *(UMB_ON_MTX.lock().await) = Some(umb_on);
    }

    // unwrap!(spawner.spawn(active_beep(pwm, None)));
    unwrap!(spawner.spawn(cli(uart)));
    unwrap!(spawner.spawn(read_battery_from_ref(&ADC_MTX, p.PB0)));

    // enable at last minute so other tasks can still spawn if can bus is down
    can.enable().await;
    let (can_tx, can_rx) = can.split();
    unwrap!(spawner.spawn(can_writer(can_tx, &CAN_TX_CHANNEL)));
    unwrap!(spawner.spawn(can_reader(can_rx, can)));

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
            let args: heapless::Vec<&str, 5> = line.split_whitespace().skip(1).collect();
            if args.len() > 1usize {
                error!("Only one argument allowed");
                continue;
            }
            match line {
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
                    // TODO: implement
                }
                "l" => {
                    // TODO: implement
                }
                "u" => {
                    // TODO: implement
                }
                "pos" => {
                    // TODO: implement
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
            Ok(_envelope) => {
                // info!("Envelope: {}", envelope);
            }
            Err(e) => {
                error!("CAN Read Error: {}", e);
                can.sleep().await;
            }
        }
    }
}
