Portland State Aerospace Society
Electromechanical Recovery System (ERS) Firmware

Overview
--------

This README file accompanies a firmware project targeting the ERS board and its STM32F091RC microcontroller, with an application written in C and built with and based on Zephyr RTOS.  The application was mostly developed in 2025 quarters 3 and 4, using Zephyr release 3.7.0 LTS.  With a couple of small modifications and the necessary newer Zephyr toolchain it also compiles with Zephyr 4.2.0, and 4.2.1.

How to build
------------

Before invoking the ERS Zephyr firmware compilation, be sure to enable your local Python virtual environment.  Assuming you are at a shell in the directory which contains the hidden ``.venv`` directory, enter the command::

    $ source ./.venv/bin/activate

ERS Zephyr based firmware is part of the PSAS repository https://github.com/psas/lv3.1-recovery.  Zephyr based firmware for the drogue and main chute firmware variants is located in ``lv3.1-recovery/controlSystem/RecoveryBoard/firmware-zephyr/``.  To compile a rough helper script to call openocd is located in ``lv3.1-recovery/controlSystem/RecoveryBoard/firmware-zephyr/``.  In your local repo change to this directory and enter the command::

    (1) $ west build -b ers-v3p1 -p -- -DBOARD_ROOT=..

Kconfig settings give a lot of insight into which features of Zephyr RTOS, MCU HAL library code and third party libraries enable for a given app.  To view ERS firmware Kconfig settings via an ncurses based interface::

    (3) $ west build -b ers-v3p1 -p -t menuconfig -- -DBOARD_ROOT=..

How to flash
------------

When the firmware build process completes with success, it is now possible to flash the app to the ERS board with the command::

    (2) $ ../flash-manually.sh w --conf-dir ../scripts

Note the ``-p`` option causes the build process to delete nearly all the prior build artifacts, and building the ``menuconfig`` option essentially limits the build to the parsing of all project Kconfig files.  After building ``menuconfig`` the most recent binaries will need to be rebuilt using command (1).

How to debug
------------

In one terminal invoke ``openocd`` with arguments::

    $ openocd -f ~/projects/psas/lv3.1-recovery/controlSystem/RecoveryBoard/zephyr/boards/others/stm32f030_demo/support/openocd.cfg

In a second terminal invoke::

    $ arm-none-eabi-gdb ./build/zephyr/zephyr.elf

Now in second terminal at the gdb prompt enter:

    (gdb) target extended-remote localhost:3333

At this point you should be able to issue the commands supported by gdb and on the respective attached MCU.  To reset and restart the firmware application:

    (gdb) monitor reset halt
    (gdb) load
    (gdb) reset

If you have a terminal program (e.g. minicom, picocom) open to the MCU debug UART you should see the given firmware logging messages appear, beginning with any boot time and start time messages.
