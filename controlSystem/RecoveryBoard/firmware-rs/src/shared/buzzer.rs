use core::sync::atomic::{AtomicBool, Ordering};

use embassy_stm32::{peripherals::TIM15, time::Hertz, timer::simple_pwm::SimplePwm};
use embassy_time::Timer;

const SEMITONE: u32 = 2_u32.pow(1 / 12); // Multiplier to shift a freq by 1 semitone

#[embassy_executor::task]
pub async fn active_beep(
    mut pwm: SimplePwm<'static, TIM15>,
    rocket_ready: Option<&'static AtomicBool>,
) {
    // Play a melody on start up
    let mut count: u8 = 0;
    pwm.ch2().enable();
    while count < 4 {
        // start at A5 and go up 1 semitone per iter
        pwm.set_frequency(Hertz(880 * ((count as u32 + 1) * SEMITONE)));
        pwm.ch2().set_duty_cycle_fully_off();
        Timer::after_millis(40).await;
        pwm.ch2().set_duty_cycle_percent(50);
        Timer::after_millis(65).await;
        count = count.wrapping_add(1);
    }
    pwm.ch2().disable();

    Timer::after_secs(1).await;

    loop {
        match rocket_ready {
            Some(rr) => {
                if rr.load(Ordering::Relaxed) {
                    pwm.ch2().enable();
                    pwm.set_frequency(Hertz(440 * 2 * SEMITONE));
                    pwm.ch2().set_duty_cycle_percent(50);
                    Timer::after_millis(50).await;
                    pwm.ch2().set_duty_cycle_fully_off();
                    pwm.set_frequency(Hertz(440 * 4 * SEMITONE));
                    pwm.ch2().set_duty_cycle_percent(50);
                    Timer::after_millis(50).await;
                    pwm.ch2().set_duty_cycle_fully_off();
                    pwm.ch2().disable();
                    Timer::after_millis(125).await;
                } else {
                    pwm.ch2().enable();
                    pwm.set_frequency(Hertz(440));
                    pwm.ch2().set_duty_cycle_percent(50);
                    Timer::after_millis(50).await;
                    pwm.ch2().set_duty_cycle_fully_off();
                    pwm.set_frequency(Hertz(440 * 3 * SEMITONE));
                    pwm.ch2().set_duty_cycle_percent(50);
                    Timer::after_millis(50).await;
                    pwm.ch2().set_duty_cycle_fully_off();
                    pwm.ch2().disable();
                    Timer::after_secs(1).await;
                }
            }
            _ => {
                pwm.ch2().enable();
                pwm.set_frequency(Hertz(440));
                pwm.ch2().set_duty_cycle_percent(50);
                Timer::after_millis(50).await;
                pwm.ch2().set_duty_cycle_fully_off();
                pwm.set_frequency(Hertz(440 * 3 * SEMITONE));
                pwm.ch2().set_duty_cycle_percent(50);
                Timer::after_millis(50).await;
                pwm.ch2().set_duty_cycle_fully_off();
                pwm.ch2().disable();
                Timer::after_secs(1).await;
            }
        }
    }
}
