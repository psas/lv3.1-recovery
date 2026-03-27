use embassy_stm32::{peripherals::IWDG, wdg::IndependentWatchdog, Peri};
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn i_wdg(i_wdg: Peri<'static, IWDG>) {
let mut i_wdg = IndependentWatchdog::new(i_wdg, 20_000_000);
    i_wdg.unleash();
    loop {
        i_wdg.pet();
        Timer::after_secs(1).await;
    }
}
