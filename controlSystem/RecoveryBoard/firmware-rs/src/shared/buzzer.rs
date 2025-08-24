use crate::shared::types::*;
use embassy_stm32::time::Hertz;
use embassy_time::{Duration, Ticker, Timer};

const SEMITONE: u32 = 2_u32.pow(1 / 12); // Multiplier to move a note by 1 semitone

#[embassy_executor::task]
pub async fn active_beep(pwm: &'static PwmType) {
    // Play a beep every 4 secs to indicate active status
    let mut ticker = Ticker::every(Duration::from_secs(4));
    loop {
        // Inner scope so that mutex unlocks automatically when not in use
        {
            let mut pwm_unlocked = pwm.lock().await;
            if let Some(pwm_ref) = pwm_unlocked.as_mut() {
                pwm_ref.ch2().enable();
                pwm_ref.set_frequency(Hertz(440));
                pwm_ref.ch2().set_duty_cycle_percent(50);
                Timer::after_millis(50).await;
                pwm_ref.ch2().set_duty_cycle_fully_off();
                pwm_ref.set_frequency(Hertz(440 * 3 * SEMITONE));
                pwm_ref.ch2().set_duty_cycle_percent(50);
                Timer::after_millis(50).await;
                pwm_ref.ch2().set_duty_cycle_fully_off();
                pwm_ref.ch2().disable();
            }
        }
        ticker.next().await;
    }
}

pub async fn play_power_on(pwm: &'static PwmType) {
    // Play a melody on start up
    {
        let mut pwm_unlocked = pwm.lock().await;
        if let Some(pwm_ref) = pwm_unlocked.as_mut() {
            let mut count: u8 = 0;
            pwm_ref.ch2().enable();
            while count < 4 {
                // start at A5 and go up 1 semitone per iter
                pwm_ref.set_frequency(Hertz(880 * ((count as u32 + 1) * SEMITONE)));
                pwm_ref.ch2().set_duty_cycle_fully_off();
                Timer::after_millis(40).await;
                pwm_ref.ch2().set_duty_cycle_percent(50);
                Timer::after_millis(65).await;
                count = count.wrapping_add(1);
            }
            pwm_ref.ch2().disable();
        }
    }
}
