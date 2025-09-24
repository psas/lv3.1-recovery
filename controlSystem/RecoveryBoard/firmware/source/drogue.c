#include <ch.h>
#include <hal.h>
#include "chprintf.h"
#include "recovery.h"

THD_WORKING_AREA(waDrogueThread, 256);

THD_FUNCTION(DrogueThread, arg) {
  (void)arg;
  msg_t drogue_line = palWaitLineTimeout(LINE_ISO_DROGUE, TIME_INFINITE);

  if (drogue_line == PAL_HIGH) {
    chprintf(DEBUG_SD, "Drogue Signal: High");
  }
}
