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
        
        palToggleLine(LINE_LED);
        chThdSleepMilliseconds(250);         
    }
    
}