/*
    Fire thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

// State      S1 S2 Case
// Locked     1  0   2
// Moving     1  1   3
// Unlocked   0  1   1
// Spinning   0  0   0

#define RING_LOCKED    2        
#define RING_MOVING    3      
#define RING_UNLOCKED  1      
#define RING_SPINNING  0      



//===========================================================================================
// Drogue!
//===========================================================================================

THD_WORKING_AREA(waDrogueThread, 256);

THD_FUNCTION(DrogueThread, arg) {

    (void)arg;
    chRegSetThreadName("drogue");
    //chprintf(DEBUG_SD, "Drogue thread starting up!\r\n");
  
    static int ringPosition = 0;
    
    // Turn on the power to the sensors TODO: power them on only when we need them.
    palSetLine(LINE_ROTSENSE_PWR);
    
    // Turn on power to motor (but keep it off) TODO: Better power management
    palClearLine(LINE_DCM_PWM);
    palClearLine(LINE_DCM_DIR);
    chThdSleepMilliseconds(10); // Wait for lines to settle
    palSetLine(LINE_DCM_PWR);
    
    while (true) {

        
        // We always need to know where we are in the ring
        ringPosition = 2*palReadLine(LINE_ROTSENSE1) + palReadLine(LINE_ROTSENSE2);

        // ----------------------------------------------------------------------------------------
        // Drogue STOP
        // ----------------------------------------------------------------------------------------
        
        if (drogueCommand == stop) {
            palClearLine(LINE_DCM_PWM); // Turn off motor
            palClearLine(LINE_DCM_DIR); // Set direction to firing
            drogueCommand = idle;       // and we're done here.
            chprintf(DEBUG_SD, "DrogueThread: Ring position = ");
            switch (ringPosition) {
                 case RING_LOCKED:
                    chprintf(DEBUG_SD, "LOCKED (10)\r\n");
                    break;
        
                case RING_MOVING:
                    chprintf(DEBUG_SD, "MOVING (11)\r\n");
                    break;
            
                case RING_UNLOCKED:
                    chprintf(DEBUG_SD, "UNLOCKED (01)\r\n");
                    break;

                case RING_SPINNING:
                    chprintf(DEBUG_SD, "SPINNING (00)\r\n");
                    break;

                default:
                    chprintf(DEBUG_SD, "DrogueThread: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
       }

        // ----------------------------------------------------------------------------------------
        // Firing Drogue
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.
        
        else if (drogueCommand == fire) {
            switch (ringPosition) {
                case RING_LOCKED:
                    chprintf(DEBUG_SD, "DrogueThread: FIRING, Position = Locked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
        
                case RING_MOVING:
                    chprintf(DEBUG_SD, "DrogueThread: FIRING, Position = Moving, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
            
                case RING_UNLOCKED:
                    chprintf(DEBUG_SD, "DrogueThread: FIRING, Position = Unlocked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;

                case RING_SPINNING:
                    chprintf(DEBUG_SD, "DrogueThread: FIRING, Position = Spinning, MOTOR OFF, \r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // OK we're done, we've past the unlock position so we're safely deployed
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    drogueCommand = idle;
                    break;

                default:
                    chprintf(DEBUG_SD, "DrogueThread: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
        }

        // ----------------------------------------------------------------------------------------
        // Locking Drogue
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.

        else if (drogueCommand == lock) {
            switch (ringPosition) {
                case RING_SPINNING:
                    chprintf(DEBUG_SD, "DrogueThread: LOCKING, Position = Spinning, MOTOR ON, \r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palSetLine(LINE_DCM_DIR); // Lock direction
                    palSetLine(LINE_DCM_PWM); // Full blast on
                    break;

                case RING_UNLOCKED:
                    chprintf(DEBUG_SD, "DrogueThread: LOCKING, Position = Unlocked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palSetLine(LINE_DCM_DIR); // Lock direction
                    palSetLine(LINE_DCM_PWM); // Full blast on
                    break;

                case RING_MOVING:
                    chprintf(DEBUG_SD, "DrogueThread: LOCKING, Position = Moving, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palSetLine(LINE_DCM_DIR); // Lock direction
                    palSetLine(LINE_DCM_PWM); // Full blast on
                    break;

                case RING_LOCKED:
                    chprintf(DEBUG_SD, "DrogueThread: LOCKING, Position = Locked, MOTOR OFF\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    palClearLine(LINE_DCM_PWM); // Set direction back to firing
                    drogueCommand = idle;
                    break;

                default:
                    chprintf(DEBUG_SD, "DrogueThread: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
        }
      
        // Check every 100 ms
        chThdSleepMilliseconds(100);        
    }
}
