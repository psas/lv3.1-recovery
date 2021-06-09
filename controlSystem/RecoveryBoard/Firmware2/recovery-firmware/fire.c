/*
    Fire thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

//===========================================================================================
// Blink!
//===========================================================================================

THD_WORKING_AREA(waFireThread, 256);

THD_FUNCTION(FireThread, arg) {

    (void)arg;
    chRegSetThreadName("fire");
    //chprintf(DEBUG_SD, "Fire thread starting up!\r\n");
  
    static int firingDrogue = FALSE;
    static int lockingDrogue = FALSE;
    static int firingMain = FALSE;
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
        // Firing Drogue
        // ----------------------------------------------------------------------------------------

        // Let us know, locally, if we should be firing
        if (fireDrogue == TRUE) {
            fireDrogue = FALSE;
            firingDrogue = TRUE; // OMG FIRE THE DROGUE
            lockingDrogue = FALSE; // Don't lock
        }
        
        // State      S1 S2 Case
        // Locked     1  0   2
        // Moving     1  1   3
        // Unlocked   0  1   1
        // Spinning   0  0   0
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.
        // TODO: HANDLE "LOCK" command
        
        if (firingDrogue == TRUE) {
            switch (ringPosition) {
                case 2: // Locked
                    chprintf(DEBUG_SD, "FireThread: Position = Locked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
        
                case 3: // Moving
                    chprintf(DEBUG_SD, "FireThread: Position = Moving, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
            
                case 1: // Unlocked
                    chprintf(DEBUG_SD, "FireThread: Position = Unlocked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;

                case 0: // Spinning
                    chprintf(DEBUG_SD, "FireThread: Position = Spinning, MOTOR OFF, \r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // OK we're done, we've past the unlock position so we're safely deployed
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    firingDrogue = FALSE;
                    break;

                default:
                    chprintf(DEBUG_SD, "FireThread: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    ringPosition = 0;
                }
        }

                // Let us know, locally, if we should be firing
        if (fireDrogue == TRUE) {
            fireDrogue = FALSE;
            firingDrogue = TRUE; // OMG FIRE THE DROGUE
        }
        
        // State      S1 S2 Case
        // Locked     1  0   2
        // Moving     1  1   3
        // Unlocked   0  1   1
        // Spinning   0  0   0
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.
        // TODO: HANDLE "LOCK" command
        
        if (firingDrogue == TRUE) {
            ringPosition = 2*palReadLine(LINE_ROTSENSE1) + palReadLine(LINE_ROTSENSE2);
            switch (ringPosition) {
                case 2: // Locked
                    chprintf(DEBUG_SD, "FireThread: Position = Locked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
        
                case 3: // Moving
                    chprintf(DEBUG_SD, "FireThread: Position = Moving, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;
            
                case 1: // Unlocked
                    chprintf(DEBUG_SD, "FireThread: Position = Unlocked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // TODO: Slow down? Reduce current? Anything to do here?
                    palClearLine(LINE_DCM_DIR); // Turn on motor full blast, unlock direction
                    palSetLine(LINE_DCM_PWM);
                    break;

                case 0: // Spinning
                    chprintf(DEBUG_SD, "FireThread: Position = Spinning, MOTOR OFF, \r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    // OK we're done, we've past the unlock position so we're safely deployed
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    firingDrogue = FALSE;
                    break;

                default:
                    chprintf(DEBUG_SD, "FireThread: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DCM_PWM); // Turn off the motor
                    ringPosition = 0;
                }
        }

        
        if (fireMain == TRUE) {
            fireMain = FALSE;
            firingMain = TRUE;
            // OMG FIRE MAIN
            // LINE_LA_POS
            // LINE_LA_DRV1
            // LINE_LA_DRV2
        }
      
        // Check every 100 ms
        chThdSleepMilliseconds(100);        
    }
}
