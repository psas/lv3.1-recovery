#ifndef _TELEMETRUM_H_
#define _TELEMETRUM_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waTelemetrumThread, 256);
extern THD_FUNCTION(TelemetrumThread, arg);

static void callback_main_high(void *arg);
static void callback_main_high(void *arg);
static void get_battery_status(void);

#endif
