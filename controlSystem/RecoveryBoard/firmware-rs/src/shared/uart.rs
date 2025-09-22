use embassy_stm32::usart::BufferedUartTx;
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, pipe::Pipe};
use embedded_io_async::Write;
use static_cell::ConstStaticCell;

pub const UART_READ_BUF_SIZE: usize = 256;

pub static TX_PIPE: Pipe<CriticalSectionRawMutex, UART_READ_BUF_SIZE> = Pipe::new();

pub static UART_TX_BUF_CELL: ConstStaticCell<[u8; UART_READ_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_READ_BUF_SIZE]);

pub static UART_RX_BUF_CELL: ConstStaticCell<[u8; UART_READ_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_READ_BUF_SIZE]);

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
