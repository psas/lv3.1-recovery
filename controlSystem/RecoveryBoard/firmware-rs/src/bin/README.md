# Board Binaries directory

The src/bin directory contains the actual binaries that get flashed to the boards.

## A Note On Interrupts

the f091rc MCU bundles all of the CAN and CEC interrupts together into one interrupt. Because of this, we must bind the Rx0, Rx1, Tx, and Sce interrupt handlers to this one interrupt.
The Embassy devs have confirmed that the handlers are written in such a way that calling them unnecessarily does nothing, so this isn't an issue.
We also bind the adc interrupt to the imported adc interrupt handler, and the Usart interrupt gets bound to the BufferedInterruptHandler, as we use a buffered usart driver in this program.

## Blinky

This binary is a simple testing program that blinks the LED and prints `"Hello, World"` over RTT.
It's purpose is to verify that your development environment is set up properly.
It can be flashed to either a NUCLEOf091rc development board or one of the actual PSAS ERS boards.

The command to flash it is `cargo run -r --bin blinky`

To flash it to an ERS pcb rather than a nucleo eval board, the BOARD environment variable must be set. Run `BOARD=main cargo -r --bin blinky `

## Parachute

This is the code for either the drogue or the main ERS parachute boards.

To flash the drogue board, run `BOARD=drogue cargo parachute`.

To flash the main board, run `BOARD=main cargo parachute`.

## Sender

This is the code for the sender board, which is the central controller for the ERS system. It monitors parachute board status, handles deployment commands, and communicates rocket readiness.

To flash, run `cargo sender`
