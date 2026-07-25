/*
The adc module defines shared analog-to-digital converter functionalities between boards.

It creates a [Watch](https://docs.rs/embassy-sync/latest/embassy_sync/watch/)
signal that holds the most recent battery reading, and the mutex that is used by the parachute board
to ensure safe access to the adc driver.

This module defines two tasks, `read_battery` and `read_battery_from_ref`.

The former is used by the sender board as it can pass the adc driver directly.

The latter is used by the parachute boards because the adc is used to both read the battery and read
the ring position in the parachute boards.
*/

use crate::types::*;
use embassy_stm32::{
    adc::{Adc, SampleTime},
    peripherals::{ADC1, PB0},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex, watch::Watch};
use embassy_time::Timer;

pub static BATT_READ_WATCH: Watch<CriticalSectionRawMutex, u8, 5> = Watch::new();

pub static ADC_MTX: AdcType = Mutex::new(None);

#[embassy_executor::task]
pub async fn read_battery(mut adc: Adc<'static, ADC1>, mut pb0: Peri<'static, PB0>) {
    loop {
        let adc_read = adc.read(&mut pb0, SampleTime::CYCLES239_5).await;
        let v_batt = ((adc_read as f32) / 4096.0 * 3.3) / 0.2326;
        let batt_sig = (v_batt / 0.1) as u8;
        BATT_READ_WATCH.sender().send(batt_sig);
        Timer::after_secs(1).await;
    }
}

#[embassy_executor::task]
pub async fn read_battery_from_ref(mut pb0: Peri<'static, PB0>) {
    loop {
        let mut batt_sig: u8 = 0;

        {
            let mut adc_unlocked = ADC_MTX.lock().await;
            if let Some(adc_ref) = adc_unlocked.as_mut() {
                let adc_read = adc_ref.read(&mut pb0, SampleTime::CYCLES239_5).await;
                let v_batt = ((adc_read as f32) / 4096.0 * 3.3) / 0.2326;
                batt_sig = (v_batt / 0.1) as u8;
            }
        }

        BATT_READ_WATCH.sender().send(batt_sig);
        Timer::after_secs(1).await;
    }
}
