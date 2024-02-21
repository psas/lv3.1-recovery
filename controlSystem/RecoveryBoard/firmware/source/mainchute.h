#ifndef _MAINCHUTE_H_
#define _MAINCHUTE_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waMainchuteThread, 256);
extern THD_FUNCTION(MainchuteThread, arg);

#endif
