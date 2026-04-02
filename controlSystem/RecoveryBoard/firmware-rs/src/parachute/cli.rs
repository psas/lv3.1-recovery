use core::fmt;
use defmt::{error, Format};
use embassy_stm32::usart::{BufferedUartRx, BufferedUartTx};
use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex,
    pipe::{self, Pipe, Reader, Writer},
};
use embedded_cli::cli::{Cli, CliBuilder, CliHandle};
use embedded_io_async::{Read, Write as AsyncWrite};
use static_cell::StaticCell;

use crate::{
    parachute::cmd::ChuteCmd,
    uart::{UartWriter, UartWriterError, MAX_CMD_LENGTH, MAX_RESPONSE_LENGTH},
};

const HISTORY_SIZE: usize = 1024;

#[derive(Debug, Format)]
pub struct CliError {}

impl core::error::Error for CliError {}

impl core::fmt::Display for CliError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "an error occured in the cli")
    }
}

impl From<UartWriterError> for CliError {
    fn from(_: UartWriterError) -> Self {
        CliError {}
    }
}
pub struct ChuteCli {
    reader: pipe::Reader<'static, CriticalSectionRawMutex, MAX_CMD_LENGTH>,
    inner_cli: Cli<&'static mut UartWriter, UartWriterError, &'static mut [u8], &'static mut [u8]>,
}

impl ChuteCli {
    pub fn process_pending_commands(
        &mut self,
        mut handler: impl FnMut(
            &mut CliHandle<&mut UartWriter, UartWriterError>,
            ChuteCmd,
        ) -> Result<(), UartWriterError>,
    ) -> Result<(), pipe::TryReadError> {
        // Read until the pipe is empty, calling handler on each command received.
        loop {
            let mut received_byte = [0u8; 1];
            let _ = self.reader.try_read(received_byte.as_mut_slice())?;

            let _ = self.inner_cli.process_byte::<ChuteCmd, _>(
                received_byte[0],
                &mut ChuteCmd::processor(|cli, command| handler(cli, command)),
            );
        }
    }
}

pub struct SerialWriteContext {
    uart_tx: BufferedUartTx<'static>,
    reader_from_cli: Reader<'static, CriticalSectionRawMutex, MAX_RESPONSE_LENGTH>,
}

pub struct SerialReadContext {
    uart_rx: BufferedUartRx<'static>,
    writer_to_cli: Writer<'static, CriticalSectionRawMutex, MAX_CMD_LENGTH>,
}

pub fn init(
    uart_tx: BufferedUartTx<'static>,
    uart_rx: BufferedUartRx<'static>,
    prompt: &'static str,
) -> Result<
    (&'static mut ChuteCli, &'static mut SerialWriteContext, &'static mut SerialReadContext),
    CliError,
> {
    static CMD_BUF: StaticCell<[u8; MAX_CMD_LENGTH]> = StaticCell::new();
    static HIST_BUF: StaticCell<[u8; HISTORY_SIZE]> = StaticCell::new();

    static UART_TO_CLI_PIPE: StaticCell<Pipe<CriticalSectionRawMutex, MAX_CMD_LENGTH>> =
        StaticCell::new();
    let uart_to_cli_pipe = UART_TO_CLI_PIPE.init(Pipe::new());
    let (uart_to_cli_reader, uart_to_cli_writer) = uart_to_cli_pipe.split();

    static CLI_TO_UART_PIPE: StaticCell<Pipe<CriticalSectionRawMutex, MAX_RESPONSE_LENGTH>> =
        StaticCell::new();
    let cli_to_uart_pipe = CLI_TO_UART_PIPE.init(Pipe::new());
    let (cli_to_uart_reader, cli_to_uart_writer) = cli_to_uart_pipe.split();

    static SERIAL_WRITE_CTX: StaticCell<SerialWriteContext> = StaticCell::new();
    let serial_write_ctx =
        SERIAL_WRITE_CTX.init(SerialWriteContext { uart_tx, reader_from_cli: cli_to_uart_reader });

    static SERIAL_READ_CTX: StaticCell<SerialReadContext> = StaticCell::new();
    let serial_read_ctx =
        SERIAL_READ_CTX.init(SerialReadContext { uart_rx, writer_to_cli: uart_to_cli_writer });

    static UART_WRITER: StaticCell<UartWriter> = StaticCell::new();
    static UART_CLI: StaticCell<ChuteCli> = StaticCell::new();

    let uart_cli = UART_CLI.init(ChuteCli {
        reader: uart_to_cli_reader,
        inner_cli: CliBuilder::default()
            .writer(UART_WRITER.init(UartWriter { writer: cli_to_uart_writer }))
            .prompt(prompt)
            .command_buffer(CMD_BUF.init([0u8; MAX_CMD_LENGTH]).as_mut_slice())
            .history_buffer(HIST_BUF.init([0u8; HISTORY_SIZE]).as_mut_slice())
            .build()?,
    });

    Ok((uart_cli, serial_write_ctx, serial_read_ctx))
}

#[embassy_executor::task]
pub async fn serial_write_task(ctx: &'static mut SerialWriteContext) {
    let mut buf = [0u8; 1];
    loop {
        ctx.reader_from_cli.read(&mut buf).await;
        if let Err(e) = AsyncWrite::write(&mut ctx.uart_tx, &buf).await {
            error!("error writing from cli: {}", e);
        }
    }
}

#[embassy_executor::task]
pub async fn serial_read_task(ctx: &'static mut SerialReadContext) {
    let mut buf = [0u8; 1];
    loop {
        if let Err(e) = ctx.uart_rx.read(&mut buf).await {
            error!("error reading from uart: {}", e);
        }
        if let Err(e) = ctx.writer_to_cli.try_write(&buf) {
            error!("error writing to cli: {}", e);
        }
    }
}
