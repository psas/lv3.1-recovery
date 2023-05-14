/*
    Blinky thread!
*/

#include "ch.h"
#include "hal.h"
#include "recovery.h"


//===========================================================================================
// Blink!
//===========================================================================================

THD_WORKING_AREA(waBlinkyThread, 256);

THD_FUNCTION(BlinkyThread, arg) {

  (void)arg;
  
  chRegSetThreadName("blinky");
  //chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
  
    while (true) {
        
        palToggleLine(LINE_LA_DRV1);

        palToggleLine(LINE_LA_DRV2);

        palToggleLine(LINE_LA_DRV12);


		palSetLine(LINE_ISO_MAIN);
		palSetLine(LINE_LA_DRV12);
		palSetLine(LINE_LA_C8);
//		palSetLine(LINE_LED);
//      palSetLine(LINE_SPKR);
		chThdSleepMilliseconds(1);
//      palClearLine(LINE_SPKR);

        if (recoveryState == armed) {
            chThdSleepMilliseconds(100);
        } else {
            chThdSleepMilliseconds(500);
        }

//        palClearLine(LINE_LED);
        palClearLine(LINE_ISO_MAIN);
        palClearLine(LINE_LA_DRV12);
        palClearLine(LINE_LA_C8);
//      palSetLine(LINE_SPKR);
//      chThdSleepMilliseconds(1);
//      palClearLine(LINE_SPKR);

        if (recoveryState == armed) {
            chThdSleepMilliseconds(100);
        } else {
            chThdSleepMilliseconds(500);
        }
          
    }
}
