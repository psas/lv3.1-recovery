PSAS Electromechanical Recovery System (ERS) board
--------------------------------------------------

Minimal readme file.  This work requires the developer to clone the lv3.1-recovery repository to some path under a Zephyr RTOS workspace.  While not enforceable this small project has been developed against Zephyr 3.7.0 LTS.  Its west manifest file calls out this Zephyr release.

Note:  if a developer's west workspace uses a different Zephyr release, then that will change in the event the developer invokes ``west update``.  Developer may wish to set up and intialize a dedicated Zephyr workspace if they have project which depend on other releases of Zephyr.

All of Zephyr's supported boards are available to the application (not yet developed) and sample apps (three in place) in this ERS firmware work.  This work entails a minimal set of board files intended to describe the ERS board.  These custom board files are subject to change as basic peripherals are proven out one at a time, to obtain the firmware "building blocks" needed for meaningful recovery board firmware.
