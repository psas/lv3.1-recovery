#![no_std]
#![no_main]

mod shared;
use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::bind_interrupts;
use embassy_stm32::can::filter::Mask32;
use embassy_stm32::can::{
    Can, Fifo, Rx0InterruptHandler, Rx1InterruptHandler, SceInterruptHandler, TxInterruptHandler
};
use embassy_stm32::gpio::{Input, Level, Output, OutputType, Pull, Speed};
use embassy_stm32::peripherals::CAN;
use embassy_stm32::time::Hertz;
use embassy_stm32::timer::low_level::CountingMode;
use embassy_stm32::timer::simple_pwm::{PwmPin, SimplePwm};
use embassy_sync::mutex::Mutex;
use shared::buzzer::{active_beep, play_power_on};
use shared::can::pingpong;
use shared::types::*;
use {defmt_rtt as _, panic_probe as _};

const CAN_BITRATE: u32 = 1_000_000;

bind_interrupts!(struct Irqs {
    CEC_CAN =>
    Rx0InterruptHandler<CAN>,
    Rx1InterruptHandler<CAN>,
    SceInterruptHandler<CAN>,
    TxInterruptHandler<CAN>;
});

static CAN_MTX: CanType = Mutex::new(None);
static PWM_MTX: PwmType = Mutex::new(None);

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

    let mut can = Can::new(p.CAN, p.PA11, p.PA12, Irqs);
    can.modify_config().set_bitrate(CAN_BITRATE);
    can.modify_filters().enable_bank(0, Fifo::Fifo0, Mask32::accept_all());
    can.enable().await;

    {
        // put drivers into mutex. inner scope so that mutex is unlocked when out of scope
        *(PWM_MTX.lock().await) = Some(pwm);
        *(CAN_MTX.lock().await) = Some(can);
    }

    // TODO: Check speeds
    let umb_on = Input::new(p.PA8, Pull::Up);
    let batt_read = Input::new(p.PB0, Pull::None);
    let serial_out = Output::new(p.PA2, Level::Low, Speed::Medium);
    let serial_in = Input::new(p.PA3, Pull::Up);
    let can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);
    let motor_ps = Output::new(p.PB6, Level::Low, Speed::Medium);
    let iso_drogue = Input::new(p.PA5, Pull::Up);
    let iso_main = Input::new(p.PA6, Pull::Down);
    let deploy_1 = Output::new(p.PB4, Level::Low, Speed::Medium);
    let deploy_2 = Output::new(p.PB5, Level::Low, Speed::Medium);

    let shore_power_is_on = batt_read.is_low();

    play_power_on(&PWM_MTX).await;

    unwrap!(spawner.spawn(active_beep(&PWM_MTX)));
    unwrap!(spawner.spawn(pingpong(&CAN_MTX)));
}
