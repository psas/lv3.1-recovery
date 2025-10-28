# Board Binaries directory

The src/bin directory contains the actual binaries that get flashed to the boards.

  - [blinky.rs](#blinky)
  - [parachute.rs](#parachute)
  - [sender.rs](#sender)

## Blinky

This binary is a simple testing program that blinks the LED and prints `"Hello, World"` over RTT.
It's purpose is to verify that your development environment is set up properly.
It can be flashed to either a NUCLEOf091rc development board or one of the actual PSAS ERS boards.
To flash it to a NUCLEO run `cargo run --bin blinky`.
To flash it to an ERS board, run `cargo run --bin blinky --features=main`

## Parachute

This binary is the program for either the drogue or the main ERS parachute boards.
To flash it as the drogue board run `cargo run --bin parachute --features=drogue`.
To flash it as the main board run `cargo run --bin parachute --features=main`.

### Constants

- **`MOTOR_DRIVE_DUR_MS`**: Duration in milliseconds to drive the motor before it is considered to have timed out. This is used in the `.drive()` method of the `Motor` struct.

- **`MOTOR_DRIVE_CURR_MA`**: Current at which to drive the motor in milliamps. Increasing this will increase the force with which the motor drives the ring.

### Interrupts

the f091rc MCU bundles all of the CAN and CEC interrupts together into one interrupt. Because of this, we must bind the Rx0, Rx1, Tx, and Sce interrupt handlers to this one interrupt.
The Embassy devs have confirmed that the handlers are written in such a way that calling them unnecessarily does nothing, so this isn't an issue. 
We also bind the adc interrupt to the imported adc interrupt handler, and the Usart interrupt gets bound to the BufferedInterruptHandler, as we use a buffered usart driver in this program.

### State

The state fields for the parachute boards are as follows:

1. `id`: U8 that's 1 if it's flashed as the drogue board, or 2 if it's flashed as the main board.
2. `ready`: Bool that will be true if shorepower is off, the battery is ok, and the sender heartbeat is good. 
3. `shore_power_status`: Bool that indicates whether shore power is on or off.
4. `sender_last_seen`: U64 timestamp of the last time the sender board heartbeat was received. 

To set the state, there is a function named `set_state()` that takes in a `ChuteStateField` enum variant. For example: 

```rust
set_state(ChuteStateField::Ready(true)).await;
```

The state is stored in a mutex so we must `.await` setting it.

### Drivers

#### PWM

The PWM driver is imported from the `embassy_timer` module. There is one channel that controls the buzzer. The counting mode and initial frequency are irrelevant for this use case.

### CAN

The CAN driver is imported from the `embassy_stm32` module.


## SENDER
