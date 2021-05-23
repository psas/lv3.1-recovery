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
    static int drogueState = 0;
    static int drogueCount = 0;
    static int mainState = 0;
    static int mainCount = 0;
  
    chRegSetThreadName("telemetrum");
    //chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
   
    while (true) {
     
        // Process the drogue line
        if (drogueState == 0) {
            if (palReadLine(LINE_ISO_DROGUE) == TRUE) {
                ++drogueCount;
                if (drogueCount > 5) {
                    drogueCount = 5;
                    drogueState = 1;
                    chprintf (DEBUG_SD, "TELEMETRUM: DROGUE DETECT!\r\n");
                    // TODO: TRIGGER DROGUE DETECT AND/OR SET GLOBAL STATE
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
                   chprintf (DEBUG_SD, "TELEMETRUM: DROGUE DETECT OFF\r\n");
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
                    // TODO: TRIGGER MAIN DETECT AND/OR SET GLOBAL STATE
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
                   chprintf (DEBUG_SD, "TELEMETRUM: MAIN DETECT OFF\r\n");
                   // TODO: Maybe let the world know there's no more main? Maybe not.
               }
           }
     
        chThdSleepMilliseconds(100); // Run this loop every 100 ms (10 Hz)
    }
}
  
