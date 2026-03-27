use defmt::Format;
use embassy_stm32::{peripherals::TIM15, time::Hertz, timer::simple_pwm::SimplePwm};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex};
use embassy_time::Timer;

#[derive(Clone, Copy, Format)]
pub enum BuzzerMode {
    High,
    Low,
    Off,
}
pub type BuzzerModeMtxType = Mutex<CriticalSectionRawMutex, Option<BuzzerMode>>;
pub static BUZZER_MODE_MTX: BuzzerModeMtxType = Mutex::new(None);

#[embassy_executor::task]
pub async fn active_beep(mut pwm: SimplePwm<'static, TIM15>) {
    // Plays a regular beep while the device is operational.
    // When RocketReady is being asserted, changes to a quicker, higher pitched beep
    pwm.ch2().enable();
    for count in 1..=4 {
        // play a start up melody
        // start at A5 and go up 1 semitone per iter
        pwm.set_frequency(Hertz(440 * count));
        pwm.ch2().set_duty_cycle_fully_off();
        Timer::after_millis(40).await;
        pwm.ch2().set_duty_cycle_percent(50);
        Timer::after_millis(65).await;
    }
    pwm.ch2().disable();

    Timer::after_secs(1).await;

    loop {
        let mut delay = 1000;
        {
            let mut mode_unlocked = BUZZER_MODE_MTX.lock().await;
            if let Some(mode) = mode_unlocked.as_mut() {
                match mode {
                    BuzzerMode::High => {
                        delay = 500;
                        pwm.ch2().enable();
                        pwm.set_frequency(Hertz(1109));
                        pwm.ch2().set_duty_cycle_percent(50);
                        Timer::after_millis(100).await;
                        pwm.ch2().set_duty_cycle_fully_off();
                        pwm.ch2().disable();
                    }
                    BuzzerMode::Low => {
                        delay = 1000;
                        pwm.ch2().enable();
                        pwm.set_frequency(Hertz(440));
                        pwm.ch2().set_duty_cycle_percent(50);
                        Timer::after_millis(75).await;
                        pwm.ch2().set_duty_cycle_fully_off();
                        pwm.ch2().disable();
                    }
                    _ => {}
                }
            }
        }
        Timer::after_millis(delay).await;
    }
}
