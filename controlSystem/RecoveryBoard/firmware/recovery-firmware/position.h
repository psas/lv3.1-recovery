#ifndef _POSITION_H_
#define _POSITION_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waPositionThread, 256);
extern THD_FUNCTION(PositionThread, arg);

#endif
