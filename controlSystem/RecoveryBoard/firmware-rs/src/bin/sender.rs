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
    gpio::OutputType,
    peripherals::{ADC1, CAN, USART2},
    time::Hertz,
    timer::{
        low_level::CountingMode,
        simple_pwm::{PwmPin, SimplePwm},
    },
    usart::{
        Config as UartConfig, DataBits, InterruptHandler as UsartInterruptHandler, Parity,
        StopBits, Uart,
    },
};
use embassy_sync::{mutex::Mutex, signal::Signal};
use firmware_rs::shared::{
    adc::read_battery,
    buzzer::{active_beep, play_power_on},
    can::pingpong,
    types::*,
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

static CAN_MTX: CanType = Mutex::new(None);
static PWM_MTX: PwmType = Mutex::new(None);
static ADC_MTX: AdcType = Mutex::new(None);
static BATT_SENSE_MTX: BattSensePinType = Mutex::new(None);
static BATT_OK_SIGNAL: BattOkSignalType = Signal::new();

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    info!("Initialized stm32");

    // Set up PWM driver and buzzer
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

    {
        // put drivers into mutex. inner scope so that mutex is unlocked when out of scope
        *(PWM_MTX.lock().await) = Some(pwm);
        *(CAN_MTX.lock().await) = Some(can);
        *(ADC_MTX.lock().await) = Some(adc);
        *(BATT_SENSE_MTX.lock().await) = Some(p.PB0);
    }

    play_power_on(&PWM_MTX).await;

    unwrap!(spawner.spawn(active_beep(&PWM_MTX)));
    unwrap!(spawner.spawn(pingpong(&CAN_MTX)));
    unwrap!(spawner.spawn(read_battery(&BATT_OK_SIGNAL, &ADC_MTX, &BATT_SENSE_MTX)));
    loop {
        let sig = BATT_OK_SIGNAL.wait().await;
        info!("Batt ok: {}", sig);
    }
}
