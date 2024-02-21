#ifndef _BLINKY_H_
#define _BLINKY_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waBlinkyThread, 256);
extern THD_FUNCTION(BlinkyThread, arg);

#endif
