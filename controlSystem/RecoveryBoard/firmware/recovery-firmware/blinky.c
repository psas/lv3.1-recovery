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
        
		palSetLine(LINE_LED_GREEN);
//		palSetLine(LINE_LED);
//      palSetLine(LINE_SPKR);
//      chThdSleepMilliseconds(1);
//      palClearLine(LINE_SPKR);

        if (recoveryState == armed) {
            chThdSleepMilliseconds(100);
        } else {
            chThdSleepMilliseconds(500);
        }

//        palClearLine(LINE_LED);
        palClearLine(LINE_LED_GREEN);
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
