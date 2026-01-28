# ERS drogue and main chute firmware commands (Zephyr version)

Hall sensor limits commands:  these are a set of ERS firmware commands
implemented in the Zephyr RTOS variant of drogue chute and main chute
ERS board firmware.  Hall commands are entered with the form:

   hall <sub-command> [reading-value]

All Hall commands require a sub-command, but only some require a value.
Values are Hall sensor readings which are measured in ADC counts.

Subcommands:

  set_defaults        : restore Hall sensor limit defaults

  set_limit_active    : set Hall limit for state "active":  hall active [s1|s2]
                       [value]

  set_limit_between   : set Hall limit for state "between":  hall between [s1|s2]
                       [value]

  set_limit_inactive  : set Hall limit for state "inactive":  hall inactive
                       [s1|s2] [value]

  set_limit_v_under   : set Hall limit for state "voltage under":  hall v_under
                       [s1|s2] [value]

  show_limits         : show Hall sensor limit values (ADC counts 0..4095)

## ERS Commands Description from shell

```
uart:~$ hall help
hall - - ERS - show and set Hall sensor limit values (in ADC counts)
Subcommands:
  save_limits         : save Hall sensor limits to flash (defaults still
                       available
  set_defaults        : restore Hall sensor limit defaults
  set_limit_active    : set Hall limit for state "active":  hall active [s1|s2]
                       [value]
  set_limit_between   : set Hall limit for state "between":  hall between [s1|s2]
                       [value]
  set_limit_inactive  : set Hall limit for state "inactive":  hall inactive
                       [s1|s2] [value]
  set_limit_v_under   : set Hall limit for state "voltage under":  hall v_under
                       [s1|s2] [value]
  show_limits         : show Hall sensor limit values (ADC counts 0..4095)
```
