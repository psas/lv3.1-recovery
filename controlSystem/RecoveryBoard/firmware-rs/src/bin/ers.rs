#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::gpio::{Input, Level, Output, Pull, Speed};
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    info!("Initialized stm32");

    let umb_on = Input::new(p.PA8, Pull::Up);
    let batt_read = Input::new(p.PB0, Pull::None);
    let buzzer = Output::new(p.PB15, Level::Low, Speed::Medium);
    let serial_out = Output::new(p.PA2, Level::Low, Speed::Medium);
    let serial_in = Input::new(p.PA3, Pull::Up);
    let can_shdn = Output::new(p.PA10, Level::Low, Speed::Medium);
    let can_silent = Output::new(p.PA9, Level::Low, Speed::Medium);
    let motor_ps = Output::new(p.PB6, Level::Low, Speed::Medium);
    let deploy_1 = Output::new(p.PB4, Level::Low, Speed::Medium);
    let deploy_2 = Output::new(p.PB5, Level::Low, Speed::Medium);
}
