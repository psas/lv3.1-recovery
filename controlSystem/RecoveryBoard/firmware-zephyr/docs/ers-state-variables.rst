// TODO [ ] Move following long comment block into a separate document file in
//  ./docs.  Add a reference here to that file, so there is one file to which
//  this source file and the keeper module header file can point.

/**
 * @brief ERS "keeper" module, acts like a bulletin board to hold shared data
 *   across the app.
 *
 * @note ERS keeper module tracks and shares most ERS board condition and state
 *   information.  State info is organized in this module in the following
 *   sets of config settings, readings and "programmatic" conditions.  These are
 *   collectively named "data groups":
 *
 * (1) battery (for recovery system)
 *    +  voltage reading
 *    +  battery "ok" minimum limit
 *
 * (2) simple digital inputs
 *    +  umbilical (shore) power connected
 *    +  ISO_DROGUE input on "Sender" ERS board only
 *    +  ISO_MAIN input on "Sender" ERS board only
 *
 * (2 1/2) analog input not categorized
 *
 * (3) locking ring
 *    +  Hall sensors 1
 *       o  reading
 *       o  v_under ADC count limit
 *       o  inactive ADC count limit
 *       o  between ADC count limit
 *       o  active ADC count limit
 *    +  Hall sensors 2
 *       o  reading
 *       o  v_under ADC count limit
 *       o  inactive ADC count limit
 *       o  between ADC count limit
 *       o  active ADC count limit
 *    +  ring state, present physical position: locked, between, unlocked, unknown
 *    +  ring position detection interval
 *
 * (4) motor related (motor for lock ring)
 *    +  Motor
 *       o  MOTOR_ISENSE analog input for current reading
 *       o  NOT_MOTOR_FAILA digital input
 *       o  NOT_MOTOR_PS output to enable H-bridge
 *       o  DAC output to control current to motor
 *       o  DEPLOY1 one of two H-bridge control signals
 *       o  DEPLOY2 two of two H-bridge control signals
 *
 * (5) CAN bus related
 *
 * (6) ERS summary state data
 *    +  ring_status
 *    +  battery_voltage (in decivolts)
 *    +  battery_ok
 *    +  shore_power_ok
 *    +  can_bus_ok
 *    +  ready_state
 *
 * (7) ERS diagnostics
 *
 * (8) Lock ring event counts stored in flash
 */

/*
------------------------------------------------------------------------
Summary of known ERS inputs 2025-09-25, subject to be updated:

Digital:

[x] !UMB_ON (PA8 = GPIO input)
[x] ISO_DROGUE (PA5 = GPIO input)      . . . "Sender" ERS board only
[x] ISO_MAIN  (PA6 = GPIO input)       . . . "Sender" ERS board only
[ ] !MOTOR_FAILA (PB7 = GPIO input)

Analog:

[x] BATT_READ (PB0 = analog input ADC_IN8)
[x] MOTOR_ISENSE (PB1 = Analog input ADC_IN9)
[x] HALL1 (PA0 = Analog input ADC_IN0)
[x] HALL2 (PA1 = Analog input ADC_IN1)

Message based inputs:

[ ] CAN module   . . . given ERS board/firmware variant will keep track
                       of one or more CAN messages, through state
                       variables.
------------------------------------------------------------------------
*/ 

