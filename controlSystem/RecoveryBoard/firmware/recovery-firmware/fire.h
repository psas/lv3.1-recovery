#ifndef _FIRE_H_
#define _FIRE_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waFireThread, 256);
extern THD_FUNCTION(FireThread, arg);

#endif
