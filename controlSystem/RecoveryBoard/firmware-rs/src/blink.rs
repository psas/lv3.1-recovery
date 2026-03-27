use embassy_stm32::{
    gpio::Output, peripherals::PB14, Peri
};
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn blink_led(led_pin: Peri<'static, PB14>) {
    let mut led =
        Output::new(led_pin, embassy_stm32::gpio::Level::High, embassy_stm32::gpio::Speed::Medium);
    loop {
        led.set_high();
        Timer::after_millis(300).await;

        led.set_low();
        Timer::after_millis(300).await;
    }
}
