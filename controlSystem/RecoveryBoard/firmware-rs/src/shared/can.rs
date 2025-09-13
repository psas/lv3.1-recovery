use defmt::*;
use embassy_stm32::can::{Can, CanTx, Frame, StandardId};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, channel::Channel};
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn echo_can(mut can: Can<'static>) -> () {
    let tx_frame = Frame::new_data(unwrap!(StandardId::new(123 as _)), &[123]).unwrap();
    can.write(&tx_frame).await;
    loop {
        let envelope = can.read().await.unwrap();
        info!("CAN frame: {=u8}", envelope.frame.data()[0],);
        can.write(&envelope.frame).await;
        Timer::after_millis(1000).await;
    }
}

#[embassy_executor::task]
pub async fn can_writer(
    mut can_tx: CanTx<'static>,
    can_tx_ch: &'static Channel<CriticalSectionRawMutex, Frame, 10>,
) -> () {
    loop {
        let frame = can_tx_ch.receive().await;
        can_tx.write(&frame).await;
    }
}
