/*
* Shared UART specfic code
* UART has been set up to work with the crate
* embedded-cli-rs https://github.com/funbiscuit/embedded-cli-rs/tree/main
*/

use defmt::Format;
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, pipe};
use embedded_io::{ErrorKind, Write as SyncWrite};
use static_cell::ConstStaticCell;

pub const UART_BUF_SIZE: usize = 128;
pub static UART_TX_BUF_CELL: ConstStaticCell<[u8; UART_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_BUF_SIZE]);
pub static UART_RX_BUF_CELL: ConstStaticCell<[u8; UART_BUF_SIZE]> =
    ConstStaticCell::new([0u8; UART_BUF_SIZE]);
pub const MAX_RESPONSE_LENGTH: usize = 1024;
pub const MAX_CMD_LENGTH: usize = 128;

pub struct UartWriter {
    pub writer: pipe::Writer<'static, CriticalSectionRawMutex, MAX_RESPONSE_LENGTH>,
}

#[derive(Debug, Format)]
pub struct UartWriterError(pipe::TryWriteError);

impl From<pipe::TryWriteError> for UartWriterError {
    fn from(value: pipe::TryWriteError) -> Self {
        Self(value)
    }
}

impl core::error::Error for UartWriterError {}

impl core::fmt::Display for UartWriterError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        core::write!(f, "an error occured within the buffer writer")
    }
}

impl embedded_io::Error for UartWriterError {
    fn kind(&self) -> ErrorKind {
        embedded_io::ErrorKind::Other
    }
}

impl embedded_io::ErrorType for UartWriter {
    type Error = UartWriterError;
}

impl SyncWrite for UartWriter {
    fn write(&mut self, buf: &[u8]) -> Result<usize, Self::Error> {
        Ok(self.writer.try_write(buf)?)
    }

    fn flush(&mut self) -> Result<(), Self::Error> {
        // For this implementation, flushing is a no-op
        Ok(())
    }
}

impl ufmt::uWrite for UartWriter {
    type Error = UartWriterError;

    fn write_str(&mut self, s: &str) -> Result<(), Self::Error> {
        self.write(s.as_bytes()).map(|_| ())
    }
}
