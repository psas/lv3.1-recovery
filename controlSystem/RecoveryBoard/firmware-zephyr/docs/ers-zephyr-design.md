# ERS Zephyr Based Application

The PSAS LV3.1-recovery branch named `ers-zephyr-work` entails a Zephyr based application to run on 


## Modules

adc.c
arbiter.c
can.c
dac.c
gpio-in.c
keeper.c
main.c
motor-control.c
pwm.c
settings-ers.c
shell-support.c

# ERS Board Inputs

## Hall Sensors

Hall sensors in the ERS board and firmware involve sensor readings, defined sensor limits and corresponding sensor states.  These sensor states in turn are used to determine lock ring position, which is itself a state of the ERS system.

The identifiers are numeric and come from C enumerations.  The symbols for
default limit values also come from an enumeration, but they take advantage of
the C enum feature that members of an enum may be assigned non-contiguous
values.  In this situation we use an enum in a slightly different way to support
easier-to-reference default limit values.

## Memory Considerations

Memory is allocated statically, and in local variables.  Largest array is a few hundred bytes used to store motor current readings measured during ring lock and ring unlock operations.  This array is decalred statically.

Overall there is no dynamic memory allocation at least in the application code itself.  (Zephyr may employ dynamic memory allocation internally.)  Further, an effort is made to keep heap allocation small.

Beyond these basic memory use choices ERS application memory has not been optimized.

Application Threads
-------------------

The ERS Zephyr based application implements a modest count of threads.  Zephyr RTOS provides for both preemptive thread and cooperative threads.  the adjective "cooperative" is in some ways confusing, as cooperative threads are scheduled more agressively than preemptive ones.  They will not be paused for example, to allow a preemptable thread to execute.

Thread priorities are expressed as small integer values, and in the ERS project they're assigned to symbols whose names end in the pattern `_THREAD_PRIORITY`.

ERS threads are as of this writing all given preemptive priority levels.  This means that each and all of these threads may at given points be interrupted by Zephyr's scheduling of a higher or same priority thread.

ERS Zephyr application threads include:

*  ADC module thread
*  arbiter thread
*  CAN bus module thread  
*  GPIO module thread
*  Zephyr default 'main' thread associated with `int main()` as entry point

Zephyr Shell for CLI
--------------------

The Zephyr shell (https://docs.zephyrproject.org/3.7.0/services/shell/index.html#) is enabled and extended for ERS application.  Zephyr's shell facility has a number of built in commands.  These may nor may not be easy to disable and remove from the set of active commands.  A couple hours' searching did not reveal a way to disable these default commands.  They are present in the ERS firmware.

To see all available shell commands a user at a serial terminal emulator need only type 'help' followed by a <RETURN> keypress.  In the case of ERS Zephyr app, ERS specific commands which have been added have the pattern '- ERS -' at the beginning of their descriptions.  Zephyr shell alphabetizes commands, and the ERS commands are named for what the are related to.  As a result ERS commands appear interspersed in the output of the 'help' command.

TODO [ ] Insert figure of Zephyr command listing here.

Shell with MCU. Debugging Side Effect

At present only one USART is broken out to pin headers on the ERS board, and there are no spare GPIOs brought out to headers.  For this reason both MCU debug statements (Zephyr logging) and the Zephyr shell share a common USART.  When a message is sent via Zephyr's logging API, it's displayed two times:  once by the logging subsystem and once again by the echo behavior of the shell.  Zephyr shell has a set of printf like APIs which display messages one time, correctly, but these require access to (or to be scoped with) the shell structure associated with the applicatin's shell.  Zephyr's LOG_INF() and related logging macros don't know about the shell nor have they a design for calling code to pass a struct or pointer to a shell instance.

Where possible shell_printf like functions are called to produce correct and cleaner output.  Message generated through calls to Zephyr's logging API appear in MCU debug output in duplicate.
