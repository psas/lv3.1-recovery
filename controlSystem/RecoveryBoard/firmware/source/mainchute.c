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
}
