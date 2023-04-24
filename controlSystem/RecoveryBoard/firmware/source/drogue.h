#ifndef _DROGUE_H_
#define _DROGUE_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waDrogueThread, 256);
extern THD_FUNCTION(DrogueThread, arg);

#endif
