use crate::shared::types::*;
use embassy_stm32::{adc::Adc, peripherals::{ADC1, PB0}, Peri};
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn read_battery(
    mut adc: Adc<'static, ADC1>,
    mut pb0: Peri<'static, PB0>,
    batt_ok: &'static BattOkSignalType,
) {
    loop {
        let adc_read = adc.read(&mut pb0).await;
        let v_batt = ((adc_read as f32) / 4096.0 * 3.3) / 0.2326;
        let batt_sig = (v_batt / 0.1) as u8;
        batt_ok.signal(batt_sig);
        Timer::after_secs(1).await;
    }
}
