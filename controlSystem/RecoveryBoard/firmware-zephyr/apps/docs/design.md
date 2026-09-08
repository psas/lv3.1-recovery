# ERS Zephyr App Design

The ERS Zephyr app for Launch Vehicle 3.1 is responsible for safe, timely
recovery parachute release. The firmware is build-time configurable for both
drogue chute and main parachute operation.

This document describes the design of parachute deploying, recovery system
firmware in the Portland State Aerospace Society's Launch Vehicle 3.1. This
firmware is based on Zephyr RTOS 3.7.0. Its development time spans summer 2025
to fall of 2026.

ERS Zephyr based firmware implements the application responsible for parachute
release timing.  There is no "ERS Sender" application written in Zephyr.

Important references, beginning with a link to the ERS firmware spec, are noted
at the end of this document.

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

Modules of the ERS Zephyr app are all fairly simple. The most involved modules
arguably are the CAN communications module, the interactive shell module, and
the arbiter. Some of the modules implement threads. Some are based on sample
apps from Zephyr 3.7.0. An overview of modules and a couple of key features
follows this paragraph. Note that 'Y' means yes or present, and '-' means not
present.

_Table 2 - ERS Zephyr module attributes_

|  Module name  | has thread | has mutex | uses k_timer | sample based |
| :-----------: | :--------: | :-------: | :----------: | :----------: |
|      ADC      |     Y      |     Y     |      -       |      Y       |
|    Arbiter    |     Y      |     -     |      -       |      -       |
|      CAN      |     Y      |     -     |      Y       |      Y       |
|      DAC      |     -      |     Y     |      -       |      Y       |
|    GPIO in    |     Y      |     -     |      -       |      Y       |
|    Keeper     |     -      |     Y     |      -       |      -       |
| Motor control |     -      |    N#1    |      -       |      -       |
|      PWM      |     Y      |    N#1    |      -       |      Y       |
|   Settings    |     -      |    N#1    |      -       |      Y       |
|  Shell (CLI)  |    N#2     |     Y     |      -       |      -       |
|  Status LED   |     -      |    N#1    |      Y       |      -       |

- An "N#1" mark means the given module likely needs mutex or other resource
  protection mechanism.
- An "N#2" mark means the module runs in a Zephyr RTOS thread context, as
  opposed to an application thread.

When it comes to a module being based on a Zephyr sample app, it may be that the
module code is significantly changed and extended beyond the sample. The quality
of being sample-based is noted here, for the frequency with which the Zephyr
RTOS project has helpful, practical samples.

The next sections, up to "Status LED module" section, detail each module.

### ADC input module

The ADC input module manages the configuration and reading of four analog input
channels. Called out in the spec, these channels cover the reading of:

- ERS battery voltage
- lock ring motor current
- Hall sensor 1 output
- Hall sensor 2 output

ADC channel configuration begins in device tree source, in
`apps/boards/ers-v3p1.overlay`. Configuration parameters including channel gain,
voltage reference, acqiure time and reading resolution are expressed in this
overlay file. Calls to Zephyr's ADC API (which in turn calls HAL code from our
microcontroller manufacturer, in this case STMicro), apply configuration
settings at app start time.

Naturally the device tree overlay file does not know the meanings of the signals
to which ADC channels are wired. The ERS Zephyr ADC module itself doesn't know
these signals by name and meaning either. This module's purpose is to set up,
and then to read ADC channels periodically. It stores these readings in the
app's run time data store module, a module called the "keeper".

TODO [ ] Determine whether the presently implemented thread is needed to perform
the ADC module channel reading and storing. A kernel timer and work item to
submit to Zephyr's system work queue might be a good fit for this work, and use
less static RAM.

### Arbiter

The ERS firmware arbiter is responsible for gathering and acting on data from
various system inputs, including ADC channel inputs, digital inputs, CAN
messages and CAN bus health status. The arbiter manifests the code which decides
when to release the parachute, either drogue chute or main chute depending on
app configuration at build time.

The arbiter also responds to certain CLI invocations, some directly and some
indirectly through the firmware's keeper module.

The arbiter implements the app's highest level logic, and depends on information
from all of the sensor facing and bus facing, communications modules.

The arbiter implements a thread with a "forever" loop construct, which provides
for adjustable, periodic reckoning of system inputs.

TODO [ ] Review arbiter responsibilities and determine whether a lighter weight
scheduling of work, e.g. timer and system work queue use would be a better
design than a thread.

### CAN communication module

The ERS CAN module handles CAN message reception and transmission. A thread
implements the reception half of controller area network communications. A
kernel work item is defined and submitted to send CAN frames onto the ERS CAN
bus. Frames to send come in two types: a status frame with key ERS board state
info, and a acknowledge frame when commands are received from the ERS sendor
board.

ERS CAN module is one of the more complex modules (though all are relatively
simple). In contrast, there is just one function in this module's public API, a
function to init the module. Nearly all other interaction with the application
occurs through CAN module queries to the application keeper module.

Notable: the CAN module distinguishes the role of the ERS board on which it is
running. Each of ERS Sender board, drogue chute board, and main chute board have
distinct CAN message IDs. These ID values identify the boards to one another on
the CAN bus, and to any party who can observe the CAN bus traffic.

### DAC output module

The digital to analog (DAC) module sets the maximum current applied to the lock
ring motor. The output of the DAC feeds into the motor driver IC, which at time
of writing is a BDS63150. DAC control is effectively applied to the motor IC's
VREF pin.

The DAC provides a public API, and implements no thread. For this module no
thread is needed.

### GPIO input module

The GPIO input module is responsible to configure general purpose, digital I/O
pins, and to periodically read their logiv levels and update the application
with their states. A thread is implemented to provide for periodic GPIO reading.

As GPIO readings are gathered, this module writes them to the application keeper
module. The readings are accessed by other modules which call the keeper, and
don't need to know about the originating source of data.

Aside from an init function, there is no further public API. None is needed.

### Keeper, of run-time state

The keeper module implements and manages a run-time data store. There is no
database involved, and there is no need for a thread. The ERS Zephyr keeper
module provides a public API, composed primarily of pairs of "set" and "get"
routines. Because the keeper's data is stored in static RAM, the data is only
maintained so long as the firmware runs without power interruptions or faults.

Run time data can be stored persistently, given that there is on-chip FLASH
memory available in the ERS board microcontroller. Another module exists to
handle the details of accessing on-chip FLASH. See this application feature, see
section on the module named "settings-ers".

In the larger design picture, a primary goal of the keeper is to support a
star-like pattern of data paths. In the absence of a central store, the paths
each module would need to establish would be more complex to represent in a
graph like diagram. In a sense there would be many kinds of data paths in place
of just two commonly used ones.

With the keeper module, we can talk about modules having a data writing path,
and a data reading path. Both of these connect to one central place. This
central data store provides a convenient way for modules like the CAN module and
the arbiter, to query all the data they need to generate status messages to the
larger system, and to determine lock ring activations.

With many modules using the keeper, an important consideration arises around
design to assure that data race conditions are avoided. Fortunately, most ERS
run time data are 32-bit integer values.

For writing to and reading from Boolean flags and 32-bit integer type data, the
keeper module makes use of Zephyr atomic types and their write and read APIs. In
a couple of places where multiple values are updated or read, a mutex is used to
protect access to those data.

Notable: comments in the keeper module use "read" and "write" to describe data
ops. Comments in the settings module make use of "store" and "retrieve", to talk
about the corresponding data ops which access data in non-volatile memory.

### motor control module

The motor control module takes care of all the digital control lines to the
motor driving IC. This module also defines and sets the default maximum current
which is applied to the motor. Given this responsibility, the motor control
module is responsible for safely driving the motor, e.g. for avoiding
over-current conditions which could damage the motor.

During motor drive operations, motor control code takes a number of motor
current readings to track what the motor is using as it spins up, moves the lock
ring, and finally approaches the physical state which is the opposite of where
the ring started. This current gathering activity is a precursor to implementing
code which shuts the motor off in case too much current is drawn.

Notable: maximum motor current is defined in motor-control.c, by the symbol
DEV_DAC_SETTING_IN_SITU.

### PWM module

The PWM module serves the sole purpose of producing audible tones, by driving
the ERS board buzzer. This module implements a thread, with one of the lower
thread priorities in the app. The thread's main loop executes a state machine,
which has two states: playing, and idle.

An API to the PWM module provides for:

- playing an audio pattern
- silencing the buzzer

Audio pattern support is limited to a simple but flexible structure. Pattern
structure members support sequences of single notes up to sixteen notes long.
Each note is described by a PWM period (a scaled version of the unit 1 / Hertz)
and a duration in milliseconds. An audio pattern also entails a Boolean flag, to
indicate whether the pattern plays one time or repeats. This structure allows
for a good variety of audio patterns while keeping the code simple.

### Settings module

The ERS settings module is responsible to copy, and to read back certain data in
non-volatile memory. No thread is needed. In this module, the terms "store" and
"retrieve" are used in a manner parallel to the keeper modules operations to
write and to read data.

Distinct from run time data, which are stored in variables, in memory the size
of the data itself, these same data when written to non-volatile memory are
stored as key-value pairs. Zephyr's settings sub-system associates string-wise
keys, or names, with each datum stored. This is not a complex activity, but
still involves some detail in terms of code development. Key names are chosen,
in ways which describe their values. An effort is made to further couple these
keynames with the sensor limit names which are described in the firmware
specification.

This document contains a section about this very name management, section "Hall
And Ring Header".

Notable: the ERS settings module aids in run time system tuning, by storing Hall
sensor limit values. These values are subject to change when there are even
small physical distance changes in the Hall sensors and the associated permanent
magnets in the lock ring assembly. This is especially true when the ring
assembly is disassembled and put back together. Without persistent storage,
these Hall sensor limits would need to be entered through the CLI on each
firmware power up.

### Shell (CLI) module

The ERS command line interface makes use of the Zephyr RTOS shell facility. The
CLI works over a UART connection. Its main purpose is to support and aid
development efforts, including but not limited to system tuning and debugging.
There is no radio link to ERS firmware, therefore ERS firmware CLI is not able
to be exercised during rocket flights.

Some keys things to note:

- A few Zephyr built-in commands are enabled, and required to be enabled
- ERS specific commands have descriptions that start with "- ERS -"
- Both Zephyr standard and app specific commands are supported with a 'help'
  option

The mention of Zephyr built-in commands goes to the fact that the ERS
specification lists needed commands, and Zephyr doesn't implement any of those
specific commands. But there are reasons to keep some Zephyr shell features
enabled, even as they make a small number of built-in Zephyr commands visible.

To turn off all but the most minimal shell features disables some helpful shell
features, such as command line completion.

Zephyr's shell facility has a built-in help command. The help command not only
lists available commands but precedes this list with some useful key bindings.
See Zephyr's shell system documentation, linked in the "references" section of
this document.

NOTICE: The ERS specification has been updated since the commands in ERS Zephyr
firmware were developed. ERS Zephyr commands do not map one-to-one with the Rust
based version of ERS firmware, and they don't precisely match the commands
listed in the spec. It would be ideal to have both ERS firmware implementations
reflect the same set of commands. There is work to do in this application to
reach that alignment of CLI commands.

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

The following sections up through "Ring commands" just show the command
descriptions which are coded into the ERS Zephyr app. These are visible by
entering the command, followed by the word "help".

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

The status LED module controls a single LED. At time of writing the status LED
module supports just two states: led heartbeat, and LED off.

There is no thread involved, as LED pattern is updated by other application
modules.

- TODO [ ] Add mutex protection to the public API.
- TODO [ ] Add additional LED blink patterns to indicate errors.

# Physical and Programmatic Elements

In addition to the factoring of code into modules, the ERS design centers around
some important physical and programatic elements which deserve some discussion
of their own. The following sections cover (at least some of) these elements.

## Lock Ring

This is an important section!

The lock ring assembly is a physical part of the LV3.1 airframe and rocket. It
entails a direct current motor and gearing, both inside and outside the motor.
These mechanical elements have strength but are subject to damage if driven with
too high a current, or from certain physical configurations (e.g. driving motor
to lock ring when ring already locked).

This section covers LV3.1 lock rings and firmware design considerations made to
drive them safely.

The lock ring assembly, of which there are two in the LV3.1 airframe, locks
sections of the rocket together which contain respectively the drogue and main
parachutes. For a typical successful flight, the lock ring for the drogue chute
section is unlocked at or shortly after apogee. This releases the drogue chute
which slows the descent of the airframe. At a further time in descent, the lock
ring for the main chute is unlocked. This releases the main chute.

If ERS firmware receives a command to lock a given ring, and that ring is
detected as "locked", the firmware does not attempt to drive the lock ring
motor. Similarly when the firmware receives a command to unlock a ring, which is
detected as "unlocked", the firmwrae does not drive the motor to unlock the
ring. The firmware assumes the ring state is true, and avoids energizing the
ring motors in those cases where they would be physically unable to turn in the
given direction.

The ERS firmware specification talks about both ring position and ring status. The section for this is titled
"Hall Sensors". The title fails to convey that this section also talks about
ring position and ring state. All are discussed there.

### Lock Ring Position versus State

The ERS firmware specification introduces both lock ring position and lock ring
state. The two terms are nearly the same, each naming a set of state-like
elements. There is however more granularity in lock ring position. Listing
possible ring positions and states called out in the specification gives:

_Table 3 - Lock ring positions and state mapping_

|      ring position       | ring state  |
| :----------------------: | :---------: |
|         unknown          | unitialized |
|  locked fully qualified  |    ---v     |
|          locked          |   locked    |
|         between          |   between   |
|         unlocked         |  unlocked   |
| unlocked fully qualified |    ---^     |
|          error           |    error    |

The practical difference between ring position and ring state, is that ring
positions locked and unlocked may be fully or partially qualified. See the
firmware specification section "Hall Sensors", and the table which maps Hall
sensor cut-off (limit) values to physical ring positions.

### Ring Position Detection

Lock ring state is crucial to shaping the behavior of the electromechanical
recovery system. Lock ring position is determined by reading two Hall sensors
per ring, and is managed across multiple application modules.

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

Figure 1 shows the flow of data to guide lock ring release during a flight. On
the bench top, the ERS firmware shell provides commands which interract with
each of these modules, to read state and to make adjustments. The shell as
mentioned, is of course, not in active use during a flight, so the data path
above is complete, in the sense of the firmware's working in its practical use
setting of a rocket flight.

### Hall And Ring Header

One of the most complex parts of the ERS specification describes and defines the
interpretations of Hall sensor readings, lock ring positions, and lock ring
states.

The file hall-and-ring.h attempts to organize the following:

- Hall sensors
- Default hall sensor limit values
- Hall sensor states
- Hall sensor related settings keynames

An effort is made also to generate consistent variable names for Hall sensor
limit values and Hall sensor states.

With two sensors, and in the app four limiting values used to determine sensor
state, and ring positions and ring states determined from these, there are a lot
of names to create and manage. The use of macros in this file has its drawbacks,
but seems better than declaring enum names and variable names each manually.
They're hard to track that way.  Multiple modules access these state data
through variable and enumeration references, so naming is again important.

A possible better strategy could focus on the creation of a C structure, to
capture Hall sensor limits (which need to be update-able at run time), and
settings keynames, and at least some other names associated with sensor states,
ring positions, and ring states.

Note: this part of the program needs work!

## References

- https://docs.google.com/document/d/1DnytDlZa1X-BaIqlIBrfcuedKocKrCpTfgMspk0twxI/edit?pli=1&tab=t.0 . . . ERS firmware specification

- https://docs.zephyrproject.org/latest/samples/subsys/shell/shell.html . . . Zephyr RTOS shell documentation

- https://github.com/psas/lv3.1-recovery/tree/ers-embassy . . . ERS Embassy based firmware project
