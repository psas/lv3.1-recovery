use defmt::error;
use embassy_stm32::usart::{BufferedUartRx, BufferedUartTx};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, pipe::Pipe};
use embedded_io_async::{Read, Write};

pub const UART_READ_BUF_SIZE: usize = 64;

#[embassy_executor::task]
pub async fn uart_writer(
    mut tx: BufferedUartTx<'static>,
    tx_pipe: &'static Pipe<CriticalSectionRawMutex, UART_READ_BUF_SIZE>,
) {
    let mut wbuf: [u8; UART_READ_BUF_SIZE];
    loop {
        wbuf = [0x00; UART_READ_BUF_SIZE];
        let bytes_read = tx_pipe.read(&mut wbuf).await;
        tx.write(&wbuf[..bytes_read]).await.unwrap();
    }
}

#[embassy_executor::task]
pub async fn uart_reader(
    mut rx: BufferedUartRx<'static>,
    tx_pipe: &'static Pipe<CriticalSectionRawMutex, UART_READ_BUF_SIZE>,
) {
    let mut rbuf: [u8; UART_READ_BUF_SIZE];
    loop {
        rbuf = [0x00; UART_READ_BUF_SIZE];
        match rx.read(&mut rbuf).await {
            Ok(len) => {
                tx_pipe.write_all(&rbuf[..len]).await;
            }
            Err(e) => {
                error!("RX ERROR: {}", e);
            }
        };
    }
}
