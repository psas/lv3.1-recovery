#ifndef _TELEMETRUM_H_
#define _TELEMETRUM_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waTelemetrumThread, 256);
extern THD_FUNCTION(TelemetrumThread, arg);

#endif
