#![no_std]
#![no_main]

#[cfg(test)]
#[embedded_test::tests]
mod tests {
    use defmt::info;
    use defmt_rtt as _;
    use embassy_stm32::{
        gpio::{Level, Output, Speed},
        Peripherals,
    };
    use embassy_time::Timer;

    #[init]
    async fn init() -> Peripherals {
        // Pass in the peripherals to each test function
        embassy_stm32::init(Default::default())
    }

    #[test]
    async fn blinky(p: Peripherals) {
        info!("Testing 123");

        let mut led = Output::new(p.PA5, Level::High, Speed::Low);

        #[cfg(feature = "main")]
        let mut led = Output::new(p.PB14, Level::High, Speed::Low);

        for i in 1..=5 {
            led.set_high();
            Timer::after_millis(250).await;
            led.set_low();
            Timer::after_millis(250).await;
        }

        assert!(true)
    }
}
