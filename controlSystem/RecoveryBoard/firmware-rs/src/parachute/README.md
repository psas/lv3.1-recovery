# Parachute
This module contains parachute board specific code.

## CLI Commands

- `help`: Display all available commands
- `state`: Print internal system state
- `batt`: Display current battery voltage
- `beep`: Toggle periodic beeping
- `version`: Display firmware version information
- `l`: Move the ring towards the lock position. Adding the `--force` flag will ignore the sensor readings and drive the motor until timeout. Adding the `--pulse` flag will drive the motor for only 100ms rather than the full duration.
- `u` Similar to l, this will move the ring towards the unlocked position instead. The `--force` and `--pulse` flags are the same as for `l`.
- `pos`: Prints the current sensor readings and ring state.
- `acts`: Print the count of motor actuations stored in flash memory
- `erase`: Erase motor actuation count data from flash memory
- `limits`: Changes the hall sensor voltage thresholds stored in flash that are used to determine ring position. Pass an argument formatted like over1,under1,active1,unactive1,over2,under2,active2,unactive2. There must be exactly 8 values separated by commas.
