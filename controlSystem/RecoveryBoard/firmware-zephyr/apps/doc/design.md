# ERS Zephyr App Design

This document describes the design of parachute deploying, recovery system firmware in the Portland State Aerospace Society's Launch Vehicle 3.1.  This firmware is based on Zephyr RTOS.  Through summer 2025 to 2026, the firmware was compiled against Zephyr 3.7.0.

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

Modules of the ERS Zephyr app are all fairly simple.  The most involved modules arguably are the CAN communications module, the interactive shell module, and the arbiter.  Some of the modules implement threads.  Some are based on sample apps from Zephyr 3.7.0 smaples.  An overview of modules and a couple of key features follows this paragraph.  Note that 'Y' means yes or present, and '-' means not present.  (The dash is a little more distinct from the letter Y):

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

For writing to and reading from Boolean flags and 32-bit integer type data, the keeper module makes use of Zephyr atomic types and their write and read APIs.  In a couple of places where multiple values are updated or read, a "keeper" API mutex is used to avoid race conditions.

Most of the ERS Zephyr modules make use of the keeper.  Modules write or "post" their data to the keeper, and read it back on demand.  Often times modules read the data posted by other modules.

Notable:  comments in the keeper module use "read" and "write" to describe data ops.  Comments in the settings module make use of "store" and "retrieve", to talk about the corresponding data ops which utilize non-volatile memory.

### PWM module

The PWM module serves the sole purpose of producing audible tones, by driving the ERS board buzzer.  This module implements a thread, with one of the lower thread priorities in the app.  The thread's main loop executes a state machine, which has two states:  playing, and idle.

An API to the PWM module provides for:

- playing an audio pattern
- silencing the buzzer

Audio pattern support is limited to a simple but flexible structure.  Pattern structure members support sequences of single notes up to sixteen notes long.  Each note is described by a PWM period (a scaled version of the unit 1 / Hertz) and a duration in milliseconds.  An audio pattern also entails a Boolean flag, to indicate whether the pattern plays one time or repeats.  This structure allows for a good variety of audio patterns while keeping the code simple.

### Settings module

The ERS settings module is responsible to copy, and to read back certain data in non-volatile memory.  No thread is needed.

### Shell (CLI) module

_Stub section_

The ERS command line interface makes use of the Zephyr RTOS shell facility.  Some keys things to note:

- A few Zephyr shell commands are enabled, and required to be enabled
- ERS speicific commands have descriptions that start with "- ERS -"
- Both Zephyr standard and app specific commands are supported with a 'help'
  option

_More details to be added here_

### Status LED module

The status LED module controls a single LED.  At time of writing the status LED module supports just two states:  led heartbeat, and LED off.

There is no thread involved, as LED pattern is updated by other application modules.

TODO [ ] Add mutex protection to the public API.
TODO [ ] Add additional LED blink patterns to indicate errors.

# Physical and Programmatic Elements

In addition to the factoring of code into modules, the ERS design centers around some important physical and programatic elements which deserve some discussion of their own.  The following sections cover these elements.

## Lock Ring

This is an important section!  The lock ring assembly, of which there are two in the LV3.1 airframe, locks sections of the rocket together which contain respectively the drogue and main parachutes.  For a typical successful flight, the lock ring for the drogue chute section is unlocked at or shortly after apogee.  This releases the drogue chute which slows the descent of the airframe.  At a further time in descent, the lock ring for the main chute is unlocked.  This releases the main chute.

If ERS firmware receives a command to lock a given ring, and that ring is detected as "locked", the firmware does not attempt to drive the lock ring motor.  Similarly when the firmware receives a command to unlock a ring, which is detected as "unlocked", the firmwrae does not drive the motor to unlock the ring.  The firmware assumes the ring state is true, and avoids energizing the ring motors in those cases where they would be physically unable to turn in the given direction.

The ERS firmware specification, a document at https://docs.google.com/document/d/1DnytDlZa1X-BaIqlIBrfcuedKocKrCpTfgMspk0twxI/edit?tab=t.0#heading=h.rg42p47rcyt5, talks about both ring position and ring status.  The section for this is titled "Hall Sensors".  The title fails to convey that this section also talks about ring position / state.  A further confusing point is that ring position and ring state are pretty much the same concept, but this is not highlighted in the specification.

It would be more clear for the spec to use one term to talk about ring state.

In this design document for Zephyr based ERS firmware, "ring state" is chosen as the preferred term for the ring's given position and state.  In real life, the ERS system may not know the position of the ring.  "Unknown" is not really a physical position, but it is a state.  This is one reason to prefer "ring state" over "ring position".

### Ring Position Detection

Lock ring state is crucial to shaping the behavior of the electromechanical recovery system.  Lock ring position is determined by reading two Hall sensors per ring, and is managed across multiple application modules.  As a clue, the following modules contain the word 'position', which is the non-preferred term for ring state:

```
./arbiter.c
./keeper.c
./motor-control.c
./shell-support.c
```

TODO [ ] Change the term "position" in code comments and variable names, where
         it makes sensor to do so for clarity and use of the preferred term
         "ring state".

The arbiter determines lock ring position based on Hall sensor inputs.

The keeper module stores and shares the latest lock ring position state.

The motor control module reads the latest determined ring position, to decide whether to move the
ring.

The Zephyr shell (CLI) module reads long ring position, and some related parameters, to report
those interactively to users.

## References

_Stub section_
