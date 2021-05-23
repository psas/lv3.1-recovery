/*
    Blinky thread!
*/

#include "ch.h"
#include "hal.h"


//===========================================================================================
// Blink!
//===========================================================================================

THD_WORKING_AREA(waBlinkyThread, 256);

THD_FUNCTION(BlinkyThread, arg) {

  (void)arg;
  chRegSetThreadName("blinky");
  //chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
  
  while (true) {
    palClearLine(LINE_LED);
//    palSetLine(LINE_SPKR);
//    chThdSleepMilliseconds(1);
//    palClearLine(LINE_SPKR);

    chThdSleepMilliseconds(100);

    palSetLine(LINE_LED);
//    palSetLine(LINE_SPKR);
//    chThdSleepMilliseconds(1);
//    palClearLine(LINE_SPKR);

    chThdSleepMilliseconds(100);
  }
}
