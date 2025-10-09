use crate::types::*;
use embassy_stm32::{
    adc::Adc,
    peripherals::{ADC1, PB0},
    Peri,
};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex, watch::Watch};
use embassy_time::Timer;

pub static BATT_READ_WATCH: Watch<CriticalSectionRawMutex, u8, 2> = Watch::new();

pub static ADC_MTX: AdcType = Mutex::new(None);

#[embassy_executor::task]
pub async fn read_battery(mut adc: Adc<'static, ADC1>, mut pb0: Peri<'static, PB0>) {
    loop {
        let adc_read = adc.read(&mut pb0).await;
        let v_batt = ((adc_read as f32) / 4096.0 * 3.3) / 0.2326;
        let batt_sig = (v_batt / 0.1) as u8;
        BATT_READ_WATCH.sender().send(batt_sig);
        Timer::after_secs(1).await;
    }
}

#[embassy_executor::task]
pub async fn read_battery_from_ref(adc: &'static AdcType, mut pb0: Peri<'static, PB0>) {
    loop {
        let mut batt_sig: u8 = 0;

        {
            let mut adc_unlocked = adc.lock().await;
            if let Some(adc_ref) = adc_unlocked.as_mut() {
                let adc_read = adc_ref.read(&mut pb0).await;
                let v_batt = ((adc_read as f32) / 4096.0 * 3.3) / 0.2326;
                batt_sig = (v_batt / 0.1) as u8;
            }
        }

        BATT_READ_WATCH.sender().send(batt_sig);
        Timer::after_secs(1).await;
    }
}
