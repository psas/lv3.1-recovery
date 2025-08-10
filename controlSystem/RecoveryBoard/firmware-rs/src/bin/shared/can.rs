use crate::shared::types::*;
use defmt::*;
use embassy_stm32::can::{Frame, StandardId};
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn pingpong(can_p: &'static CanType) -> () {
    {
        let mut can_unlocked = can_p.lock().await;
        if let Some(can_ref) = can_unlocked.as_mut() {
            let tx_frame = Frame::new_data(unwrap!(StandardId::new(123 as _)), &[123]).unwrap();
            can_ref.write(&tx_frame).await;

            info!("Echoing CAN bus");
            loop {
                let envelope = can_ref.read().await.unwrap();
                info!("frame {=u8}", envelope.frame.data()[0],);
                Timer::after_millis(1000).await;
                can_ref.write(&envelope.frame).await;
            }
        }
    }
}
