# ERS Zephyr App Design

The ERS Zephyr app for Launch Vehicle 3.1 is responsible for safe, timely recovery parachute release.  The firmware is build-time configurable for both drogue chute and main parachute operation.

This document describes the design of parachute deploying, recovery system firmware in the Portland State Aerospace Society's Launch Vehicle 3.1.  This firmware is based on Zephyr RTOS 3.7.0.  Its development time spans summer 2025 to fall of 2026.

## Modules

The firmware includes modules to manage various system activies.

_Table 1: ERS Zephyr firmware modules_

- ADC input module
- Arbiter
- CAN communication module
- DAC output module
- GPIO input module
- Keeper, for run-time state
- Motor control module
- PWM module
- Settings module, for persistent state
- Shell (CLI) module
- Status LED module

Modules of the ERS Zephyr app are all fairly simple.  The most involved modules arguably are the CAN communications module, the interactive shell module, and the arbiter.  Some of the modules implement threads.  Some are based on sample apps from Zephyr 3.7.0.  An overview of modules and a couple of key features follows this paragraph.  Note that 'Y' means yes or present, and '-' means not present.  (The dash is a little more distinct from the letter Y):

_Table 2 - ERS Zephyr module attributes_

|  Module name  | has thread | has mutex | uses k_timer | sample based |
|    :----:     |   :----:   |  :----:   |    :----:    |    :----:    |
| ADC           |     Y      |     Y     |      -       |      Y       |
| Arbiter       |     Y      |     -     |      -       |      -       |
| CAN           |     Y      |     -     |      Y       |      Y       |
| DAC           |     -      |     Y     |      -       |      Y       |
| GPIO in       |     Y      |     -     |      -       |      Y       |
| Keeper        |     -      |     Y     |      -       |      -       |
| Motor control |     -      |     N#1   |      -       |      -       |
| PWM           |     Y      |     N#1   |      -       |      Y       |
| Settings      |     -      |     N#1   |      -       |      Y       |
| Shell (CLI)   |     N#2    |     Y     |      -       |      -       |
| Status LED    |     -      |     N#1   |      Y       |      -       |

- An "N#1" mark means the given module likely needs mutex or other resource
  protection mechanism.
- An "N#2" mark means the module runs in a Zephyr RTOS thread context, as
  opposed to an application thread.

When it comes to a module being based on a Zephyr sample app, it may be that the module code is significantly changed and extended beyond the sample.  The quality of being sample-based is noted here, for the frequency with which the Zephyr RTOS project has helpful, practical samples.

### ADC input module

The ADC input module manages the configuration and reading of four analog input channels.  Called out in the spec, these channels cover the reading of:

- ERS battery voltage
- lock ring motor current
- Hall sensor 1 output
- Hall sensor 2 output

ADC channel configuration begins in device tree source, in ``apps/boards/ers-v3p1.overlay``.  Configuration parameters including channel gain, voltage reference, acqiure time and reading resolution are expressed in this overlay file.  Calls to Zephyr's ADC API (which in turn calls HAL code from our microcontroller manufacturer, in this case STMicro), apply configuration settings at app start time.

Naturally the device tree overlay file does not know the meanings of the signals to which ADC channels are wired.  The ERS Zephyr ADC module itself doesn't know these signals by name and meaning either.  This module's purpose is to set up, and then to read ADC channels periodically.  It stores these readings in the app's run time data store module, a module called the "keeper".

TODO [ ] Determine whether a thread is needed to perform the ADC module channel reading and storing.  A kernel timer and work item to submit to Zephyr's system work queue might be a good fit for this work, and use less static RAM.

### Arbiter

The ERS firmware arbiter is responsible for gathering and acting on data from various system inputs, including ADC channel inputs, digital inputs, CAN messages and CAN bus health status.  The arbiter manifests the code which decides when to release the parachute, either drogue chute or main chute depending on app configuration at build time.

The arbiter also responds to certain CLI invocations, some directly and some indirectly through the firmware's keeper module.

The arbiter implements the app's highest level logic, and depends on information from all of the sensor facing and bus facing, communications modules.

The arbiter implements a thread with a "forever" loop construct, which provides for adjustable, periodic reckoning of system inputs.

TODO [ ] Review monitor responsibilities and determine whether a lighter weight scheduling of work, e.g. timer and system work queue use would be a better design than a thread.

TODO [ ] locate following paragraph in CLI section:

  These occur over a UART connection during firmware and some hardware development, and are helpful for tuning and testing on the bench top.  There is no radio link to ERS firmware, therefore ERS firmware CLI is not able to be exercised during rocket flights.

### CAN communication module

The ERS CAN module handles CAN message reception and transmission.  A thread implements the reception half of controller area network communications.  A kernel work item is defined and submitted to send CAN frames onto the ERS CAN bus.  Frames to send come in two types:  a status frame with key ERS board state info, and a acknowledge frame when commands are received from the ERS sendor board.

ERS CAN module is one of the more complex modules (though all are relatively simple).  In contrast, there is just one function in this module's public API, a function to init the module.  Nearly all other interaction with the application occurs through CAN module queries to the application keeper module.

Notable:  the CAN module distinguishes the role of the ERS board on which it is running.  Each of ERS Sender board, drogue chute board, and main chute board have distinct CAN message IDs.  These ID values identify the boards to one another on the CAN bus, and to any party who can observe the CAN bus traffic.

### DAC output module

The digital to analog (DAC) module sets the maximum current applied to the lock ring motor.  The output of the DAC feeds into the motor driver IC, which at time of writing is a BDS63150.  DAC control is effectively applied to the motor IC's VREF pin.

The DAC provides a public API, and implements no thread.  For this module no thread is needed.

### GPIO input module

The GPIO input module is responsible to configure general purpose, digital I/O pins, and to periodically read their logiv levels and update the application with their states.  A thread is implemented to provide for periodic GPIO reading.

Aside from an init function, there is no further public API.  None is needed.

### motor control module

The motor control module takes care of all the digital control lines to the motor driving IC.  This module also defines and sets the default maximum current which is applied to the motor.  Given this responsibility, the motor control module is responsible for safely driving the motor, and for avoiding over-current conditions which could damage the motor.

During motor drive operations, motor control code takes a number of motor current readings to track what the motor is using as it spins up, moves the lock ring, and finally approaches the physical state which is the opposite of where the ring started.  This current gathering activity is a precursor to implementing code which shuts the motor off in case too much current is drawn.

Notable:  maximum motor current is defined in motor-control.c, by the symbol DEV_DAC_SETTING_IN_SITU.

### Keeper, of run-time state

The keeper module implements and manages a run-time data store.  There is no database involved, and there is no need for a thread.  The ERS Zephyr keeper module provides a public API, composed primarily of pairs of "set" and "get" routines.  Because the keeper's data is stored in static RAM, the data is only maintained so long as the firmware runs without power interruptions or faults.

Run time data can be stored persistently.  See the section on the module named settings-ers.

In the larger design picture, a primary goal of the keeper module design is to support a star-like pattern for data to travel in the application.  Modules need to share data with each other.  In the absence of a central store, the paths each module would need to establish would be more complex to represent in a graph like diagram.  In a sense there would be many kinds of data paths in place of just one or two.  With the keeper module, we can talk about modules having a data writing path, and a data reading path, to one central place, when they need one or both paths.  The keeper module also provides a convenient way for modules like the CAN module and the arbiter, to query all the data they need to generate status messages to the larger system, and to determine lock ring activations.

With many modules using the keeper, an important consideration arises around design to assure that data race conditions are avoided.  Fortunately, most ERS run time data are 32-bit integer values.

For writing to and reading from Boolean flags and 32-bit integer type data, the keeper module makes use of Zephyr atomic types and their write and read APIs.  In a couple of places where multiple values are updated or read, a mutex is used to protect access to those data.

Notable:  comments in the keeper module use "read" and "write" to describe data ops.  Comments in the settings module make use of "store" and "retrieve", to talk about the corresponding data ops which access data in non-volatile memory.

### PWM module

The PWM module serves the sole purpose of producing audible tones, by driving the ERS board buzzer.  This module implements a thread, with one of the lower thread priorities in the app.  The thread's main loop executes a state machine, which has two states:  playing, and idle.

An API to the PWM module provides for:

- playing an audio pattern
- silencing the buzzer

Audio pattern support is limited to a simple but flexible structure.  Pattern structure members support sequences of single notes up to sixteen notes long.  Each note is described by a PWM period (a scaled version of the unit 1 / Hertz) and a duration in milliseconds.  An audio pattern also entails a Boolean flag, to indicate whether the pattern plays one time or repeats.  This structure allows for a good variety of audio patterns while keeping the code simple.

### Settings module

The ERS settings module is responsible to copy, and to read back certain data in non-volatile memory.  No thread is needed.  In this module, the terms "store" and "retrieve" are used in a manner parallel to the keeper modules operations to write and to read data.

Distinct from run time data, which are stored in variables, in memory the size of the data itself, these same data when written to non-volatile memory are stored as key-value pairs.  Zephyr's settings sub-system associates string-wise keys, or names, with each datum stored.  This is not a complex activity, but still involves some detail in terms of code development.  Key names need to be selected, and it is helpful to associate them with their values . . .




### Shell (CLI) module

The ERS command line interface makes use of the Zephyr RTOS shell facility.  Some keys things to note:

- A few Zephyr shell commands are enabled, and required to be enabled
- ERS speicific commands have descriptions that start with "- ERS -"
- Both Zephyr standard and app specific commands are supported with a 'help'
  option

NOTICE:  The ERS specification has been updated since the commands in ERS Zephyr firmware were developed.  These commands do not map one-to-one with the Rust based version of ERS firmware.  It would be ideal to have both ERS firmware implementations reflect the same set of commands.  There is work to do in this application to reach that alignment of CLI commands.

The Zephyr shell, or shell sub-system, has a built-in help command.  The help command not only lists available commands but precedes these with some useful key bindings.  Available commands vary from app to app, depending on what the application implements beyond Zephyr's built in commands.  See Zephyr's shell system documentation, linked in the "references" section of this document.

_Table n - ERS shell commands_

```
uart:~$ help
Please press the <Tab> button to see all available commands.
You can also use the <Tab> button to prompt or auto-complete all commands or its subcommands.
You can try to call commands with <-h> or <--help> parameter for more information.

Shell supports following meta-keys:
  Ctrl + (a key from: abcdefklnpuw)
  Alt  + (a key from: bf)
Please refer to shell documentation for more details.

Available commands:
  can     : CAN controller commands
  dac     : - ERS - DAC info and set commands
  device  : Device commands
  diag    : - ERS - diagnostics
  ers     : - ERS - development commands
  hall    : - ERS - show and set Hall sensor limit values (in ADC counts)
  help    : Prints the help message.
  kernel  : Kernel commands
  led     : - ERS - status LED
  motor   : - ERS - motor use info
  ring    : - ERS - lock ring commands
  stats   : Stats commands
```

_More details to be added here_

#### DAC commands

```
uart:~$ dac help
dac - - ERS - DAC info and set commands
Subcommands:
  range                     : show microcontroller DAC range
  show_present_value        : show present DAC setting
  set                       : set DAC output
  set_lock_unlock_current   : set DAC value to limit lock ring motor current
  show_lock_unlock_current  : show DAC value to limit lock ring motor current
```

#### Diagnostics commands

```
uart:~$ diag help
diag - - ERS - diagnostics
Subcommands:
  on   : enable ERS periodic diagnostics
  off  : disable ERS periodic diagnostics
```

#### development commands

```
uart:~$ ers help
ers - - ERS - development commands
Subcommands:
  adcall  : Read ERS board's four ADC channels
  adc0    : Read ERS board ADC for Hall sensor 1
  adc1    : Read ERS board ADC for Hall sensor 2
```

#### Hall sensor commands

```
uart:~$ hall help
hall - - ERS - show and set Hall sensor limit values (in ADC counts)
Subcommands:
  active        : set Hall limit for state "active":  hall active [s1|s2] [value]
  between       : set Hall limit for state "between":  hall between [s1|s2]
                 [value]
  inactive      : set Hall limit for state "inactive":  hall inactive [s1|s2]
                 [value]
  retrieve      : retrieve Hall sensor limits from flash
  save          : store Hall sensor limits to flash (default limits still
                 available)
  set_defaults  : restore Hall sensor limit defaults
  show          : show Hall sensor limit values (ADC counts 0..4095)
  v_under       : set Hall limit for state "voltage under":  hall v_under [s1|s2]
                 [value]
```

#### LED commands

```
uart:~$ led help
led - - ERS - status LED
Subcommands:
  on   : enable ERS status LED
  off  : disable ERS status LED
```

#### Motor commands

```
uart:~$ motor help
motor - - ERS - motor use info
Subcommands:
  show  : show motor use count
  set   : set motor use count (for when motor is replaced)
```

#### Ring commands

```
uart:~$ ring help
ring - - ERS - lock ring commands
Subcommands:
  diset     : set ring position detection internal in ms
  dishow    : show ring position detection internal in ms
  lock      : lock ring
  position  : show lock ring position
  state     : show lock ring state
  unlock    : unlock ring
```

### Status LED module

The status LED module controls a single LED.  At time of writing the status LED module supports just two states:  led heartbeat, and LED off.

There is no thread involved, as LED pattern is updated by other application modules.

- TODO [ ] Add mutex protection to the public API.
- TODO [ ] Add additional LED blink patterns to indicate errors.

# Physical and Programmatic Elements

In addition to the factoring of code into modules, the ERS design centers around some important physical and programatic elements which deserve some discussion of their own.  The following sections cover these elements.

## Lock Ring

This is an important section!

The lock ring assembly is a physical part of the LV3.1 airframe and rocket.  It entails a direct current motor and gearing, both inside and outside the motor.  These mechanical elements have strength but are subject to damage if driven with too high a current, or from certain physical configurations (e.g. driving motor to lock ring when ring already locked).

This section covers LV3.1 lock rings and firmware design considerations made to drive them safely.

The lock ring assembly, of which there are two in the LV3.1 airframe, locks sections of the rocket together which contain respectively the drogue and main parachutes.  For a typical successful flight, the lock ring for the drogue chute section is unlocked at or shortly after apogee.  This releases the drogue chute which slows the descent of the airframe.  At a further time in descent, the lock ring for the main chute is unlocked.  This releases the main chute.

If ERS firmware receives a command to lock a given ring, and that ring is detected as "locked", the firmware does not attempt to drive the lock ring motor.  Similarly when the firmware receives a command to unlock a ring, which is detected as "unlocked", the firmwrae does not drive the motor to unlock the ring.  The firmware assumes the ring state is true, and avoids energizing the ring motors in those cases where they would be physically unable to turn in the given direction.

The ERS firmware specification, a document at https://docs.google.com/document/d/1DnytDlZa1X-BaIqlIBrfcuedKocKrCpTfgMspk0twxI/edit?tab=t.0#heading=h.rg42p47rcyt5, talks about both ring position and ring status.  The section for this is titled "Hall Sensors".  The title fails to convey that this section also talks about ring position and ring state.

### Lock Ring Position versus State

The ERS firmware specification introduces both lock ring position and lock ring state.  The two terms are nearly the same, each naming a set of state-like elements.  There is however more granularity in lock ring position.  Listing possible ring positions and states called out in the specification gives:

_Table 3 - Lock ring positions and state mapping_

| ring position            | ring state  |
| :---:                    | :---:       |
| unknown                  | unitialized |
| locked fully qualified   | ---v        |
| locked                   | locked      |
| between                  | between     |
| unlocked                 | unlocked    |
| unlocked fully qualified | ---^        |
| error                    | error       |

The practical difference between ring position and ring state, is that ring positions locked and unlocked may be fully or partially qualified.  See the firmware specification section "Hall Sensors", and the table which maps Hall sensor cut-off (limit) values to physical ring positions.

### Ring Position Detection - a programmatic design

Lock ring state is crucial to shaping the behavior of the electromechanical recovery system.  Lock ring position is determined by reading two Hall sensors per ring, and is managed across multiple application modules.

_Figure 1 - Data path Hall sensor readings to lock ring motor movement_

```
 +-----------------+   +--------------+   +-------------+   +-----------------+
 |   ADC module    |-->|    Keeper    |-->|   Arbiter   |-->|  Motor control  |
 +-----------------+   +--------------+   +-------------+   +-----------------+
```

- ADC module gathers Hall sensor readings
- Keeper stores the readings for other modules to use
- Arbiter determines lock ring position based on readings and Hall limit values
- Motor control module actuates the lock ring motor  

Figure 1 shows the flow of data to guide lock ring release during a flight.  On the bench top, the ERS firmware shell provides commands which interract with each of these modules, to read state and to make adjustments.  The shell as mentioned, is of course, not in active use during a flight, so the data path above is complete for the firmware's most important work and and inner working.

## References

- https://docs.zephyrproject.org/latest/samples/subsys/shell/shell.html
