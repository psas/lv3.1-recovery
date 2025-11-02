# Shared Modules

This README explains all modules in the src directory. The modules are used by both the `parachute.rs` and `sender.rs` binaries.
- [adc.rs](#Adc)
- [blink.rs](#Blink)
- [buzzer.rs](#Buzzer)
- [can.rs](#Can)
- [motor.rs](#Motor)
- [ring.rs](#Ring)
- [types.rs](#Types)
- [uart.rs](#Uart)

## Adc

The adc module defines shared analog-to-digital converter functionalities between boards. It creates a [Watch](https://docs.rs/embassy-sync/latest/embassy_sync/watch/struct.Watch.html) signal that holds the most recent battery reading, and the mutex that is used by the parachute board to ensure safe access to the adc driver.

This module defines two tasks, `read_battery` and `read_battery_from_ref`. The former is used by the sender board, as it can pass the adc driver directly, and the latter is used by the parachute boards, because they need to use the mutex. This is because the adc is used to both read the battery and read the ring position in the parachute boards.

## Blink

This module defines the task that blinks the LED on all 3 boards to indicate operational status.

## Buzzer

The buzzer module manages the audible feedback that all 3 boards use to indicate their status. It uses an enum to determine which mode it should be in: High, Low, or Off. It uses PWM to control the buzzer, and thus depends on a PWM driver to be passed in by the binaries. When the task begins, it plays a short melody. Then it enters a loop that unlocks the mutex that contains its mode enum. It matches the mode enum and will play a quicker, higher-pitched beep if its in High mode, or a less frequent, lower pitched beep if its in Low mode. Otherwise, it does nothing.

## Can

The can module defines several constants, creates a [channel](#https://docs.rs/embassy-sync/latest/embassy_sync/channel/struct.Channel.html), defines the `CanTxChannelMsg` struct, and defines two tasks, `echo_can` and `can_writer`.

### Constants

- **`CAN_BITRATE: u32 = 1_000_000`**
  The CAN bus communication speed set to 1 Mbps (1,000,000 bits per second). This high-speed configuration ensures rapid message delivery between all ERS system components.

- **`DROGUE_DEPLOY_ID: u16 = 0x100`**
  Command message ID for deploying the drogue parachute. Sent by the sender board to initiate drogue release.

- **`DROGUE_ACKNOWLEDGE_ID: u16 = 0x101`**
  Acknowledgment message ID sent by the drogue parachute board to confirm it received and is executing the deployment command.

- **`DROGUE_HEARTBEAT_ID: u16 = 0x710`**
  Status message ID used by the drogue parachute board to broadcast its current state (ready, motor status, sensor readings, etc.).

- **`MAIN_DEPLOY_ID: u16 = 0x200`**
  Command message ID for deploying the main parachute. Sent by the sender board to initiate main parachute release.

- **`MAIN_ACKNOWLEDGE_ID: u16 = 0x201`**
  Acknowledgment message ID sent by the main parachute board to confirm it received and is executing the deployment command.

- **`MAIN_HEARTBEAT_ID: u16 = 0x720`**
  Status message ID used by the main parachute board to broadcast its current state (ready, motor status, sensor readings, etc.).

- **`SENDER_HEARTBEAT_ID: u16 = 0x700`**
  Periodic status message ID used by the sender board to broadcast overall system health, including:
  - Rocket readiness status
  - Battery voltage and health
  - Shore power status
  - Parachute board communication status
  - Deployment signal status

The message IDs follow a numbering scheme:
- **0x1XX**: Drogue parachute commands and acknowledgments
- **0x2XX**: Main parachute commands and acknowledgments
- **0x7XX**: System status and heartbeat messages

### Tasks

- `echo_can`
  Debug task that will simply write a message to the bus, and then enter a loop that waits for a response which it then echoes back to the bus again. To be used with `echo_can.py`.

- `can_writer`
  Handles writing messages to the CAN bus for all boards. The channel used by this task only accepts a CanTxChannelMsg, which is a simple wrapper around Embassy's CAN Frame, that tells the writer task if it should write the message with a blocking write, or if it should only try to write the message and fail with an error if it can't.

## Motor

The motor driver module provides control and management of the parachute deployment motor system. It handles motor direction, current limiting, and position-based control with safety features.

### Constants

- **`MOTOR_DRIVE_CURR_MA`**: Maximum current limit for motor operation (2000 mA)
- **`MOTOR_DRIVE_DUR_MS`**: Default timeout duration for motor operations

### Struct Fields

- `deploy1`: Output pin for motor direction control (forward)
- `deploy2`: Output pin for motor direction control (reverse)
- `ps`: Power save control pin
- `motor_fail`: Input pin for motor failure detection
- `dac`: DAC peripheral for current limiting control

### `MotorMode`

Enum defining the possible motor operating modes:

- `PowerSave`: Minimum power consumption mode
- `Stop`: Motor stopped but powered
- `Forward`: Drive motor forward (toward unlocked position)
- `Reverse`: Drive motor reverse (toward locked position)
- `Brake`: Active braking mode

### Methods

#### `Motor::new()`

Creates a new motor controller instance.

**Parameters:**
- `pb4`: GPIO pin for deploy1 control
- `pb5`: GPIO pin for deploy2 control
- `pb6`: GPIO pin for power save control
- `pb7`: GPIO pin for motor failure input
- `dac`: DAC peripheral for current limiting

#### `set_mode()`

Sets the motor operating mode by controlling the output pins.

**Parameters:**
- `mode`: `MotorMode` enum value specifying desired operation

#### `limit_motor_current()`

Configures the DAC to limit motor current to the specified value.

**Parameters:**
- `ma`: Desired current limit in milliamps (0-2000 mA)

**Implementation Details:**
- Uses circuit analysis to convert mA to DAC value
- Formula: `scale = (ma * 1024) / 1375`
- Sets 12-bit right-aligned DAC value for current control

#### `read_ring_pos_until_condition()`

Monitors ring position and motor current until target position is reached.

**Parameters:**
- `position`: Target `RingPosition` to wait for

**Behavior:**
- Continuously reads ring position and motor current sensors
- Writes out CSV of `MOTOR_ISENSE` pin values over defmt rtt.
- Returns when target position is detected

#### `drive()`

Main motor driving function with comprehensive control and safety features.

**Parameters:**
- `mode`: Target `RingPosition` (Locked or Unlocked)
- `duration_ms`: Maximum drive time in milliseconds
- `force`: Whether to ignore sensor feedback
- `current`: Motor current limit in milliamps

**Operation:**
1. Sets current limit via DAC
2. Stops motor initially
3. Drives in specified direction based on target position
4. Monitors either position feedback or uses timeout
5. Returns to power save mode when complete

## Ring

The ring position sensor module provides real-time monitoring and position detection for the parachute deployment ring system. It uses hall effect sensors to determine the ring's position (locked, unlocked, or intermediate states) and monitors motor current consumption.

### Statics

- **`RingType`**: Thread-safe mutex wrapper for ring controller: `Mutex<ThreadModeRawMutex, Option<Ring>>`

- **`RING_MTX`**: Global mutex for ring controller access

- **`RING_POSITION_WATCH`**: Broadcasts current `RingPosition` state
- **`SENSOR_READ_WATCH`**: Broadcasts raw sensor readings from both hall sensors
- **`MOTOR_ISENSE_WATCH`**: Broadcasts motor current sense readings

### Enums

#### `RingPosition`

Defines the possible states of the parachute deployment ring:

- **`Locked`**: Ring is in the locked position (parachute secured)
- **`Unlocked`**: Ring is in the unlocked position (parachute deployed)
- **`Inbetween`**: Ring is between locked and unlocked positions
- **`Error`**: Invalid or unexpected sensor readings detected

#### `SensorState`

Internal state machine for individual sensor interpretation:

- **`Active`**: Sensor reading indicates active/magnet-present state
- **`Unactive`**: Sensor reading indicates inactive/no-magnet state
- **`Under`**: Reading below minimum valid threshold
- **`Over`**: Reading above maximum valid threshold
- **`Inbetween`**: Reading in transition zone between states

### Structures

#### `SensorReadings`

Container for raw ADC readings from both hall sensors:

**Fields:**
- `sensor1`: Raw ADC value from first hall sensor (PA0)
- `sensor2`: Raw ADC value from second hall sensor (PA1)

**Methods:**
- `new(sensor1, sensor2)`: Creates new sensor readings instance

#### `SensorLimits`

Calibration thresholds for interpreting sensor readings:

**Fields:**
- `over`: Upper threshold for "over-range" detection
- `under`: Lower threshold for "under-range" detection
- `active`: Threshold for active sensor state
- `unactive`: Threshold for inactive sensor state

**Methods:**
- `new(over, under, active, unactive)`: Creates calibrated limit set

#### `Ring Struct`

Main ring position controller managing sensor hardware and position detection:

**Fields:**
- `pa0`: First hall effect sensor input
- `pa1`: Second hall effect sensor input
- `pb1`: Motor current sense input
- `sensor1_limits`: Calibration limits for sensor 1
- `sensor2_limits`: Calibration limits for sensor 2

#### Methods

##### `Ring::new()`

Creates a new ring position controller with calibrated sensor limits.

**Parameters:**
- `pa0`: First hall sensor peripheral
- `pa1`: Second hall sensor peripheral
- `pb1`: Motor current sense peripheral

**Calibration Values:**
- **Sensor 1**: Over=3700, Under=600, Active=2100, Unactive=900
- **Sensor 2**: Over=3700, Under=600, Active=1300, Unactive=900

**NOTE**: Calibration are experimentally derived and may need to be changed.

##### `broadcast_ring_position()`

Main position detection routine that reads sensors, interprets position, and broadcasts updates.

**Operation Flow:**
1. Acquires ADC mutex for sensor reading access
2. Reads all three analog inputs:
   - Sensor 1 (PA0) - Hall effect position
   - Sensor 2 (PA1) - Hall effect position
   - Motor current sense (PB1)
3. Broadcasts raw readings via watch signals
4. Interprets sensor states using calibrated thresholds
5. Determines ring position based on sensor state combination
6. Broadcasts final ring position

### Internal Helper Functions

#### `get_sensor_state()`

Converts raw ADC values to discrete sensor states using calibrated limits.

**Parameters:**
- `adc_val`: Raw 12-bit ADC reading (0-4095)
- `limit`: `SensorLimits` structure with calibration thresholds

**State Determination Logic:**
- **Over**: `adc_val >= limit.over`
- **Under**: `adc_val <= limit.under` 
- **Active**: `adc_val >= limit.active && adc_val < limit.over`
- **Unactive**: `adc_val <= limit.unactive && adc_val > limit.under`
- **Inbetween**: Values between active/unactive thresholds

#### `get_ring_position()`

Determines overall ring position based on combined sensor states.

**Position Logic:**
- **Unlocked**: 
  - Sensor1 Active AND Sensor2 not Active
  - Sensor1 not Unactive AND Sensor2 Unactive
- **Locked**:
  - Sensor1 not Active AND Sensor2 Active  
  - Sensor1 Unactive AND Sensor2 not Unactive
- **Inbetween**: Either sensor in Inbetween state
- **Error**: All other combinations

### Tasks

#### `read_pos_sensor`

Background task that continuously monitors ring position at 50ms intervals.

**Operation:**
1. Acquires ring controller mutex
2. Calls `broadcast_ring_position()` to update all watch signals
3. Waits 50ms before next reading
4. Runs indefinitely in loop

## Types

This module contains various type definitions that may be used in more than one place.

## Uart

Provides buffered UART communication with async I/O traits implementation for CLI interface. Used by CLI task for command-line interface with noline crate compatibility.

### Constants

- **`UART_BUF_SIZE: usize = 1024`**
  Size of transmit and receive buffers (1KB each)

### Static Resources

See [static cell](https://docs.rs/static_cell/latest/static_cell/)

#### Buffer Allocation

```
UART_TX_BUF_CELL  // Transmit buffer static cell
UART_RX_BUF_CELL  // Receive buffer static cell
```

**Purpose:** Statically allocated buffers with compile-time known sizes for zero-allocation UART operation.

### Structures

#### `IO`

UART I/O wrapper that implements embedded-io-async traits.

**Fields:**
- `stdio`: Buffered UART driver instance

**Methods:**
- `new(stdio)`: Creates new IO wrapper

#### `Error`

Simple error type for I/O operations.

#### Traits Implementation

##### `embedded_io_async::Read`

- **`read()`**: Reads data from UART into buffer
- Returns bytes read or I/O error

##### `embedded_io_async::Write`

- **`write()`**: Writes buffer data to UART
- **`flush()`**: Ensures all data transmitted
- Returns bytes written or I/O error
