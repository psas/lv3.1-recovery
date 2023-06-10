#ifndef _BEEP_H_
#define _BEEP_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waBeepThread, 256);
extern THD_FUNCTION(BeepThread, arg);

#endif
