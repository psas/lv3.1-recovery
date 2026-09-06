# ERS Zephyr App Design

This document describes the design of parachute deploying, recovery system firmware in the Portland State Aerospace Society's Launch Vehicle 3.1.  This firmware is based on Zephyr RTOS.  Through summer 2025 to 2026, the firmware was compiled against Zephyr 3.7.0.

## Modules

The firmware includes modules to manage various system activies.

_Table 1: ERS Zephyr firmware modules_

- ADC input module
- CAN communication module
- DAC output module
- motor control
- PWM module, for ERS system buzzer
- settings module, for run-time and persistent state
- shell (CLI) module
- status LED module


### PWM module

The PWM module serves the sole purpose of producing audible tones, by driving the ERS board buzzer.  This module implements a thread, with one of the lower thread priorities in the app.  The thread's main loop executes a state machine, which has two states:  playing, and idle.

An API to the PWM module provides for:

- playing an audio pattern
- silencing the buzzer

Audio pattern support is limited to a simple but flexible structure.  Pattern structure members support sequences of single notes up to sixteen notes long.  Each note is described by a PWM period (a scaled version of the unit 1 / Hertz) and a duration in milliseconds.  An audio pattern also entails a Boolean flag, to indicate whether the pattern plays one time or repeats.  This structure allows for a good variety of audio patterns while keeping the code simple.

# Physical and Programmatic Elements

In addition to the factoring of code into modules, the ERS design centers around some important physical and programatic elements which deserve some discussion of their own.  The following sections cover these elements.

## Lock Ring

This is an important section!  The lock ring assembly, of which there are two in the LV3.1 airframe, locks sections of the rocket together which contain the drogue and main parachutes.  For a typical successful flight, the lock ring for the drogue chute section is unlocked at or shortly after apogee.  This releases the drogue chute which slows the descent of the airframe.  At a further time in descent, the lock ring for the main chute is unlocked.  This releases the main chute.

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
