use embassy_stm32::usart::BufferedUart;
use embedded_io_async::ErrorKind;
use static_cell::ConstStaticCell;

pub const UART_BUF_SIZE: usize = 1024;

pub static UART_TX_BUF_CELL: ConstStaticCell<[u8; UART_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_BUF_SIZE]);

pub static UART_RX_BUF_CELL: ConstStaticCell<[u8; UART_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_BUF_SIZE]);

pub struct IO {
    pub stdio: BufferedUart<'static>,
}

impl IO {
    pub fn new(stdio: BufferedUart<'static>) -> Self {
        Self { stdio }
    }
}

#[derive(Debug)]
pub struct Error(());

impl embedded_io_async::Error for Error {
    fn kind(&self) -> ErrorKind {
        ErrorKind::Other
    }
}

impl embedded_io_async::ErrorType for IO {
    type Error = Error;
}

impl embedded_io_async::Read for IO {
    async fn read(&mut self, buf: &mut [u8]) -> Result<usize, Self::Error> {
        match self.stdio.read(buf).await {
            Ok(bytes_read) => Ok(bytes_read),
            Err(_) => Err(self::Error(())),
        }
    }
}

impl embedded_io_async::Write for IO {
    async fn write(&mut self, buf: &[u8]) -> Result<usize, Self::Error> {
        match self.stdio.write(buf).await {
            Ok(bytes_written) => {
                Ok(bytes_written)
            }
            Err(_) => Err(self::Error(())),
        }
    }

    async fn flush(&mut self) -> Result<(), Self::Error> {
        if (self.stdio.flush().await).is_err() {
            Err(Error(()))
        } else {
            Ok(())
        }
    }
}
