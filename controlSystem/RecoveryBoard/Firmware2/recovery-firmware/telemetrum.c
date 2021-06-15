/*
    Telemetrum thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

//===========================================================================================
// Telemtrum - monitor the lines coming in from the Telemetrum. Debounce the lines, and 
//             let the world know if we see a valid transition.
//===========================================================================================

THD_WORKING_AREA(waTelemetrumThread, 256);

THD_FUNCTION(TelemetrumThread, arg) {

    (void)arg;
    
    static int powerState = 0;  // Keep track of the shore power state locally.
    static int powerCount = 0;    
    static int drogueState = 0; // Keep track of drogue line from the Telemetrum locally.
    static int drogueCount = 0;
    static int mainState = 0;   // Keep track of main line from the Telemetrum locally.
    static int mainCount = 0;
  
    chRegSetThreadName("telemetrum");
    //chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
   
    while (true) {

        // Process the shore power line. Broadcast any change in shore power state. Disarm if shore power is on, and /*arm */if it's off.
        if (powerState == 0) {
            if (palReadLine(LINE_SHORE_PWR) == 0) {
                ++powerCount;
                if (powerCount > 5) {
                    powerCount = 5;
                    powerState = 1;
                    chprintf (DEBUG_SD, "SHORE POWER: On!\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    if (recoveryState == armed) {
                        chprintf (DEBUG_SD, "SHORE POWER: Disarming!\r\n");
                        chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                        recoveryState = disarmed;
                    }
                }
            }
            else {
                powerCount = 0; // Nope, it wasn't 5 in a row, reset to zero if there was any previous powerCount
            }
        }
        else // powerState == 1
           if (palReadLine(LINE_SHORE_PWR) == 0) {
               powerCount = 5; // Nope, it wasn't 5 in a row, reset back to 5 if there was any previous powerCount
           }
           else {
               --powerCount;
               if (powerCount < 1) {
                   powerCount = 0;
                   powerState = 0;
                    chprintf (DEBUG_SD, "SHORE POWER: Off!\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    if (recoveryState == disarmed) {
                        chprintf (DEBUG_SD, "SHORE POWER: arming!\r\n");
                        chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                        recoveryState = armed;
                    }
               }
           }
        
        
        // Process the drogue line
        if (drogueState == 0) {
            if (palReadLine(LINE_ISO_DROGUE) == TRUE) {
                ++drogueCount;
                if (drogueCount > 5) {
                    drogueCount = 5;
                    drogueState = 1;
                    chprintf (DEBUG_SD, "TELEMETRUM: DROGUE DETECT!\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    if (recoveryState == armed) {
                        chprintf (DEBUG_SD, "TELEMETRUM: DROGUE FIRING!\r\n");
                        chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                        drogueCommand = fire;
                    }
                }
            }
            else {
                drogueCount = 0; // Nope, it wasn't 5 in a row, reset to zero if there was any previous drogueCount
            }
        }
        else // drogueState == 1
           if (palReadLine(LINE_ISO_DROGUE) == TRUE) {
               drogueCount = 5; // Nope, it wasn't 5 in a row, reset back to 5 if there was any previous drogueCount
           }
           else {
               --drogueCount;
               if (drogueCount < 1) {
                   drogueCount = 0;
                   drogueState = 0;
                   chprintf (DEBUG_SD, "TELEMETRUM: DROGUE OFF\r\n");
                   // TODO: Maybe let the world know there's no more drogue? Maybe not.
               }
           }
     
        // Process the main line
        if (mainState == 0) {
            if (palReadLine(LINE_ISO_MAIN) == TRUE) {
                ++mainCount;
                if (mainCount > 5) {
                    mainCount = 5;
                    mainState = 1;
                    chprintf (DEBUG_SD, "TELEMETRUM: MAIN DETECT!\r\n");
                    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                    if (recoveryState == armed) {
                        chprintf (DEBUG_SD, "TELEMETRUM: MAIN FIRING!\r\n");
                        chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                        mainchuteCommand = fire_m;
                    }
                }
            }
            else {
                mainCount = 0; // Nope, it wasn't 5 in a row, reset to zero if there was any previous mainCount
            }
        }
        else // mainState == 1
           if (palReadLine(LINE_ISO_MAIN) == TRUE) {
              mainCount = 5; // Nope, it wasn't 5 in a row, reset back to 5 if there was any previous mainCount
           }
           else {
               --mainCount;
               if (mainCount < 1) {
                   mainCount = 0;
                   mainState = 0;
                   chprintf (DEBUG_SD, "TELEMETRUM: MAIN OFF\r\n");
                   // TODO: Maybe let the world know there's no more main? Maybe not.
               }
           }
     
        chThdSleepMilliseconds(100); // Run this loop every 100 ms (10 Hz)
    }
}
  
