use crate::shared::types::*;
use defmt::info;
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn read_battery(
    batt_ok: &'static BattOkSignalType,
    adc: &'static AdcType,
    pb0: &'static BattSensePinType,
) {
    {
        let mut adc_unlocked = adc.lock().await;
        let mut batt_sense_pin_unlocked = pb0.lock().await;
        loop {
            {
                if let Some(adc_ref) = adc_unlocked.as_mut() {
                    if let Some(batt_sense_pin_ref) = batt_sense_pin_unlocked.as_mut() {
                        let adc_read = adc_ref.read(batt_sense_pin_ref).await;
                        let v_batt = (adc_read as f32) / 4096.0 * 3.3 / 0.2326;
                        info!("v_batt: {}", v_batt);
                        let is_batt_ok = v_batt > 10.5;
                        batt_ok.signal(is_batt_ok);
                    }
                }
            }
            Timer::after_secs(1).await;
        }
    }
}
