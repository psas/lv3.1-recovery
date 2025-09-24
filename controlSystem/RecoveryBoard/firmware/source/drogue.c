#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "recovery.h"

THD_WORKING_AREA(waDrogueThread, 256);

THD_FUNCTION(DrogueThread, arg) {
  (void)arg;
  chRegSetThreadName("Drogue");
  msg_t drogue_line = palWaitLineTimeout(LINE_ISO_DROGUE, TIME_INFINITE);

  if (drogue_line == MSG_OK) {
    chprintf(DEBUG_SD, "Drogue Signal: High");
  }
  // don't care for these, just want to check if any signal is returned
  if (drogue_line == MSG_RESET) {
    chprintf(DEBUG_SD, "Drogue Signal: Reset");
  }
  if (drogue_line == MSG_TIMEOUT) {
    chprintf(DEBUG_SD, "Drogue Signal: TIMEOUT");
  }
}
