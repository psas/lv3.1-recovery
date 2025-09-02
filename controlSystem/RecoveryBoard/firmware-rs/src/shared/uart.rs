use defmt::error;
use embassy_stm32::{
    mode::Async,
    usart::{UartRx, UartTx},
};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, pipe::Pipe};

pub const READ_BUF_SIZE: usize = 8; // Uart Read Buffer Size

#[embassy_executor::task]
pub async fn uart_writer(
    mut tx: UartTx<'static, Async>,
    tx_pipe: &'static Pipe<CriticalSectionRawMutex, READ_BUF_SIZE>,
) {
    let mut wbuf: [u8; READ_BUF_SIZE];
    loop {
        wbuf = [0x00; READ_BUF_SIZE];
        let _bytes_read = tx_pipe.read(&mut wbuf).await;
        embedded_io_async::Write::write(&mut tx, &wbuf).await.unwrap();
    }
}

#[embassy_executor::task]
pub async fn uart_reader(
    mut rx: UartRx<'static, Async>,
    tx_pipe: &'static Pipe<CriticalSectionRawMutex, READ_BUF_SIZE>,
) {
    let mut rbuf: [u8; READ_BUF_SIZE];
    loop {
        rbuf = [0x00; READ_BUF_SIZE];
        match rx.read_until_idle(&mut rbuf).await {
            Ok(_) => {
                tx_pipe.write_all(&rbuf).await;
            }
            Err(e) => {
                error!("RX ERROR: {}", e);
            }
        };
    }
}
