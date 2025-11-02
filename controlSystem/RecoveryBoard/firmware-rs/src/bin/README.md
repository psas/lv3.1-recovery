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

### Interrupts

the f091rc MCU bundles all of the CAN and CEC interrupts together into one interrupt. Because of this, we must bind the Rx0, Rx1, Tx, and Sce interrupt handlers to this one interrupt.
The Embassy devs have confirmed that the handlers are written in such a way that calling them unnecessarily does nothing, so this isn't an issue. 
We also bind the adc interrupt to the imported adc interrupt handler, and the Usart interrupt gets bound to the BufferedInterruptHandler, as we use a buffered usart driver in this program.

### State

The state fields for the parachute boards are as follows:

- `id`: U8 that's 1 if it's flashed as the drogue board, or 2 if it's flashed as the main board.

- `ready`: Bool that will be true if shorepower is off, the battery is ok, and the sender heartbeat is good. 

- `shore_power_status`: Bool that indicates whether shore power is on or off.

- `sender_last_seen`: U64 timestamp of the last time the sender board heartbeat was received. 

To set the state, there is a function named `set_state()` that takes in a `ChuteStateField` enum variant. For example: 

```rust
set_state(ChuteStateField::Ready(true)).await;
```

The state is stored in a mutex so we must `.await` setting it.

#### GPIO

- PA8: Umbilical status input
- PA10: CAN shutdown control
- PA9: CAN silent mode control

### Drivers

#### PWM

The PWM driver is imported from the `embassy_timer` module. It use pin B15 configured as a `PwmPin`, and is controlled by the TIM15 timer. There is one channel that controls the buzzer. The counting mode and initial frequency are irrelevant for this use case, but must be set anyway.

#### CAN

The CAN driver is imported from the `embassy_stm32` module. It used the CAN peripheral, pins A11 and A12, and the `CanIrqs` we created when binding the interrupt handlers to the `CEC_CAN` interrupt. The filter bank is set up to accept all CAN messages, but could be configured with a mask to only accept certain message IDs. However, there are currently so few messages on the bus that it really isnt necessary to use the filters at this point. The only relevant portion of the config is the bitrate. It is set to `CAN_BITRATE`, which is imported from the CAN module located in the src dir.

#### ADC

The ADC driver is imported from the `embassy_stm32` module. It uses the ADC1 peripheral and the `AdcIrqs` struct we created when binding the interrupt handlers to the `ADC1_COMP` interrupt. The sample time and resolution are currenntly set to the max possible values to maximize the accuracy of the adc readings. There is no reason to change these settings.

#### UART

The UART driver is imported from the `embassy_stm32` module, and configured with a baudrate of 115200, No parity, 8 data bits, and 1 Stop bit (8N1). It takes the `USART2` peripheral, pin A3 for RX, pin A2 for TX, TX and RX static cell buffers, the `UsartIrqs` struct we created when binding the `BufferedInterruptHandler` to the `USART2` interrupt. The tx/rx buffers must be created using static cells because they must have static lifetimes. More info [here](https://docs.rs/static_cell/latest/static_cell/).

#### DAC

The DAC driver is imported from the `embassy_stm32` module, and created using the DAC1 peripheral. DAC1 Ch1 uses DMA1_CH3 and the associated pin is A4. Ch1 could also use DMA2_CH3 instead if needed. DAC1 Ch2 uses DMA1_CH4 and the associated pin is A5. Ch2 could also use DMA2_CH4 instead if needed.

#### MOTOR

The motor driver is imported from the local `motor.rs` module. It uses pins B4 for deploy1, B5 for deploy2, B6 for MOTOR_PS, and B7 for MOTOR_FAILA. It also takes a static mutable reference to the dac which doesn't need to be put into a mutex because it is not shared with other tasks.

#### Ring

The ring driver is imported from the local `ring.rs` module. It's mainly responsible for reading the position of the ring and determining if it's locked or unlocked using two hall sensors. Therefore, it takes in pins A0, A1, and PB1. These are the two hall sensors, and the motor_isense, respectively.

### Tasks

Tasks in Embassy are similar to threads. They allow different functions to be ran essentially concurrently. See the [embassy docs](https://embassy.dev/book/) for more info. The parachute boards have a total of 6 tasks that the exector manages.

#### `blink_led`

Blinks the LED on pin B14 on and off regularly to indicate the board is functional.

#### `active_beep`

PWM's the buzzer on startup and regularly during operation to indicate the board is functional.

#### `cli`

Manages user input over UART via a cli-like interface. Several commands are available.

- `help`: Display all available commands and a short explanation of each.

- `state`: Print the internal state.

- `l`: Move the ring towards the lock position. Adding the `--force` flag will ignore the sensor readings and drive the motor until timeout. Adding the `--pulse` flag will drive the motor for only 100ms rather than the full duration.

- `u` Similar to l, this will move the ring towards the unlocked position instead. The `--force` and `--pulse` flags are the same as for `l`.

- `pos`: Prints the current sensor readings and ring state. Adding `--poll` will repeatedly print the readings and ring state until the user interrupts it with any input.

The cli task uses the [noline](https://docs.rs/noline/latest/noline/) crate to manage its interface.

#### `read_battery_from_ref`

This task is imported from the local `adc.rs` module. It unlocks the mutex containing the reference to the adc and sends the reading out over the `BATT_READ_WATCH` signaling primitive. More info on what a Watch is [here](https://docs.embassy.dev/embassy-sync/git/default/watch/struct.Watch.html).

#### `read_pos_sensor`

The ring struct has a method named `broadcast_ring_position` on it that will update the `RING_POSITION_WATCH` signal with the latest position of the ring: locked, unlocked, inbetween, or error. This task is imported from the local `ring.rs` module. It repeatedly unlocks the mutex that contains the reference to the ring struct and calls `broadcast_ring_position` every 50ms.

#### `can_writer`

The `can_writer` task is imported from the local `can.rs` module. It waits for a CAN message to come in over the `CAN_TX_CHANNEL` and will write the message to the CAN bus. A channel is similar to a watch, with a few key differences that make it more appropriate for this task. It's documentation can be found [here](https://docs.embassy.dev/embassy-sync/git/default/channel/struct.Channel.html). The channel only accepts a `CanTxChannelMsg`, which is a simple wrapper around Embassy's CAN Frame, that tells the writer task if it should write the message with a blocking write, or if it should only try to write the message and fail with an error if it can't.

#### `can_reader`

`can_reader` is the task that handles waiting for CAN messages to come in on the CAN bus and responding appropriately. It behaves differently depending on whether the board was flashed with `--features=drogue` or `--features=main`.

- If the task receives a message with an ID matching `DROGUE_DEPLOY_ID`, and it was flashed with the drogue feature flag, it will unlock the motor mutex and drive the motor to unlock the ring. If it was flashed with the main feature flag, it will do nothing.

- If the task receives a message with an ID matching `MAIN_DEPLOY_ID`, and it was flashed with the main feature flag, it will unlock the motor mutex and drive the motor to unlock the ring. Otherwise, it will do nothing.

- If the task receives a message with an ID matching `SENDER_HEARTBEAT_ID`, it will update its internal `sender_last_seen` state field with the timestamp of the message.

#### `parachute_heartbeat`

This task sends a message over the CAN bus that communicates its state and whether its ready to drive the motor or not. Because it also reads many bits of state, it also updates the system state at the same time. This presents potential for improvement, as this isn't really what the heartbeat task should do. It should only be responsible for sending the CAN message. Unfortunately, due to time constraints this refactoring hasn't been a high priority. In a perfect world `main()` would be responsible for business logic such as updating important state fields, setting the buzzer's mode, printing messages when state changes, etc, and the heartbeat task would simply communicate this state to the CAN bus.

It accomplishes sending this message mainly by constructing a status buffer that contains u8 representations of its internal state. It grabs receivers for the various Watch signals and unlocks global mutexes as needed and then conditionally sends messages based on which feature flag the board was flashed with.

## Sender

This binary is the program for the sender board, which acts as the central controller for the ERS system. It monitors parachute board status, handles deployment commands, and communicates rocket readiness.

### Interrupts

The sender board uses the same interrupt bundling approach as the parachute boards:
- CAN and CEC interrupts are bundled into the `CEC_CAN` interrupt
- ADC uses the `ADC1_COMP` interrupt handler
- UART uses the `BufferedInterruptHandler` for the `USART2` interrupt

### State

The sender board maintains comprehensive system state:

- `rocket_ready`: Bool indicating if all systems are go for launch
- `force_rocket_ready`: Bool that can override normal readiness checks
- `drogue_status`: Bool indicating drogue parachute board health
- `main_status`: Bool indicating main parachute board health
- `shore_power_status`: Bool indicating if shore power is connected
- `drogue_last_seen`: U64 timestamp of last drogue status message
- `main_last_seen`: U64 timestamp of last main status message
- `iso_drogue_last_seen`: U64 timestamp of last drogue deployment signal
- `iso_main_last_seen`: U64 timestamp of last main deployment signal

State is managed through a `set_state()` function that takes `SenderStateField` enum variants:

```rust
set_state(SenderStateField::RocketReady(true)).await;
```

### Drivers

#### PWM

The PWM driver controls the buzzer using TIM15 timer and pin PB15 configured as a `PwmPin`. It generates audible alerts for system status.

#### CAN

The CAN driver uses the CAN peripheral with pins PA11 and PA12, configured with a bitrate of `CAN_BITRATE`. The filter bank accepts all CAN messages since message filtering is handled in software.

#### ADC

The ADC driver uses ADC1 peripheral with maximum sample time and 12-bit resolution for accurate battery voltage monitoring on pin PB0.

#### UART

The UART driver uses USART2 peripheral with pins PA2 (TX) and PA3 (RX), configured for 115200 baud, 8N1 format. It uses buffered operation with static cells for TX/RX buffers.

#### GPIO

Multiple GPIO pins are used for critical functions:
- PA8: Umbilical status input
- PA6: Main parachute deployment signal input  
- PA5: Drogue parachute deployment signal input
- PA7: Rocket Ready output signal
- PA10: CAN shutdown control
- PA9: CAN silent mode control

### Tasks

#### `blink_led`

Blinks the LED on pin PB14 to indicate the board is operational.

#### `active_beep`

Controls the buzzer to provide audible status indications. Different beep patterns indicate system states like rocket readiness.

#### `cli`

Provides a command-line interface over UART for system monitoring and control:

- `help`: Display all available commands
- `state`: Print internal system state
- `drogue`: Send drogue deployment command
- `main`: Send main deployment command  
- `rr`: Toggle Rocket Ready signal override
- `batt`: Display current battery voltage
- `beep`: Toggle periodic beeping
- `version`: Display firmware version information

#### `read_battery`

Continuously monitors battery voltage via ADC and updates the `BATT_READ_WATCH` signal.

#### `handle_iso_rising_edge`

Listens for deployment signals from the telemetrum on two separate tasks (one for drogue, one for main). When a rising edge is detected, it verifies umbilical status and initiates the corresponding parachute deployment.

#### `can_writer`

Handles transmission of CAN messages from the `CAN_TX_CHANNEL`. Uses the same `CanTxChannelMsg` structure as parachute boards to support both blocking and non-blocking writes.

#### `can_reader`

Processes incoming CAN messages:
- Updates parachute board status and timestamps from status messages
- Records deployment acknowledgments from parachute boards
- Tracks communication health with parachute boards

#### `telemetrum_heartbeat`

The core system monitoring task that:
- Evaluates overall system readiness based on parachute board status, battery level, and shore power
- Controls the Rocket Ready output signal
- Sends periodic status messages over CAN
- Updates buzzer mode based on system state
- Monitors for state changes and logs them

This task integrates multiple system aspects to determine if the rocket is ready for launch, considering factors like battery voltage, parachute board communication health, and manual overrides. Similarly to the parachute boards, a potential source of improvement would be refactoring the heartbeat to communicate the state rather than handling business logic that should be handled by `main()`.
