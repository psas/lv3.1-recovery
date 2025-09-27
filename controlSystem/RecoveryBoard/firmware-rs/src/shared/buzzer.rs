use defmt::info;
use embassy_stm32::{peripherals::TIM15, time::Hertz, timer::simple_pwm::SimplePwm};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex};
use embassy_time::Timer;

pub enum BuzzerMode {
    High,
    Low,
    Off,
}

pub type BuzzerModeMtxType = Mutex<CriticalSectionRawMutex, Option<BuzzerMode>>;

#[embassy_executor::task]
pub async fn active_beep(mut pwm: SimplePwm<'static, TIM15>, mode: &'static BuzzerModeMtxType) {
    let mut count: u8 = 0;

    // start up melody
    pwm.ch2().enable();
    while count < 4 {
        // start at A5 and go up 1 semitone per iter
        pwm.set_frequency(Hertz(440 * (count as u32 + 1)));
        pwm.ch2().set_duty_cycle_fully_off();
        Timer::after_millis(40).await;
        pwm.ch2().set_duty_cycle_percent(50);
        Timer::after_millis(65).await;
        count = count.wrapping_add(1);
    }
    pwm.ch2().disable();

    Timer::after_secs(1).await;

    loop {
        let mut delay: u64 = 777; // Picking delays that hopefully won't overlap other tasks
        {
            let mut mode_unlocked = mode.lock().await;
            if let Some(mode) = mode_unlocked.as_mut() {
                match mode {
                    BuzzerMode::High => {
                        delay = 587;
                        pwm.ch2().enable();
                        pwm.set_frequency(Hertz(1109));
                        pwm.ch2().set_duty_cycle_percent(50);
                        Timer::after_millis(100).await;
                        pwm.ch2().set_duty_cycle_fully_off();
                        pwm.ch2().disable();
                    }
                    BuzzerMode::Low => {
                        delay = 1137;
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
