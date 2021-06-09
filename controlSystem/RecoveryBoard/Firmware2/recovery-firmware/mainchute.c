/*
    Fire thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

//===========================================================================================
// Mainchute!
//===========================================================================================

THD_WORKING_AREA(waMainchuteThread, 256);

THD_FUNCTION(MainchuteThread, arg) {

    (void)arg;
    chRegSetThreadName("mainchute");
    //chprintf(DEBUG_SD, "mainchute thread starting up!\r\n");
  
    static int actuatorPosition = 0;
     
    // Make sure the LA is off
    palClearLine(LINE_LA_DRV1);
    palClearLine(LINE_LA_DRV2);
    
    while (true) {

        // We always need to know where we are in the LA
        // Linear Actuator potentiometer is on PA1 = ADC_IN1
        actuatorPosition = 0; // TODO ADC READ HERE

        // ----------------------------------------------------------------------------------------
        // Main STOP
        // ----------------------------------------------------------------------------------------
        
        if (mainchuteCommand == stop_m) {
            palClearLine(LINE_LA_DRV1);
            palClearLine(LINE_LA_DRV2);
            mainchuteCommand = idle_m;       // and we're done here.
            chprintf(DEBUG_SD, "MainchuteThread: Actuator position = %d", actuatorPosition);
       }
       
        // ----------------------------------------------------------------------------------------
        // Firing Main
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LA speed, and check for motor jamming. Try going backwards if necessary.
        
        else if (mainchuteCommand == fire_m) {
            if (actuatorPosition > 500) {
                chprintf(DEBUG_SD, "MainchuteThread: FIRING, Position = %d/3300, MOTOR DOWN\r\n",actuatorPosition);
                palClearLine(LINE_LA_DRV1);
                palSetLine(LINE_LA_DRV2);
            }
            else {
                chprintf(DEBUG_SD, "MainchuteThread: FIRING, Position = %d/3300, MOTOR OFF\r\n",actuatorPosition);
                chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                // OK we're done, we've past the unlock position so we're safely deployed
                palClearLine(LINE_LA_DRV1);
                palClearLine(LINE_LA_DRV2);
                mainchuteCommand = idle_m;       // and we're done here.
            }
        }

        // ----------------------------------------------------------------------------------------
        // Reseting Main
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LA speed, and check for motor jamming. Try going backwards if necessary.

        else if (mainchuteCommand == reset_m) {
            if (actuatorPosition < 2800) {
                chprintf(DEBUG_SD, "MainchuteThread: RESETING, Position = %d/3300, MOTOR UP\r\n",actuatorPosition);
                palSetLine(LINE_LA_DRV1);
                palClearLine(LINE_LA_DRV2);
            }
            else {
                chprintf(DEBUG_SD, "MainchuteThread: RESETING, Position = %d/3300, MOTOR OFF\r\n",actuatorPosition);
                chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                palClearLine(LINE_LA_DRV1);
                palClearLine(LINE_LA_DRV2);
                mainchuteCommand = idle_m;       // and we're done here.
            }
        }
        
        // Check every 100 ms
        chThdSleepMilliseconds(100);        
    }
}
