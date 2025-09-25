#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

THD_WORKING_AREA(waDrogueThread, 256);

THD_FUNCTION(DrogueThread, arg) {
  (void)arg;
  chRegSetThreadName("Drogue");
  palEnableLineEvent(LINE_ISO_DROGUE, PAL_EVENT_MODE_RISING_EDGE);
  while (true) {
    chThdSleepMilliseconds(10);  // Wait for printout (100 char ~ 10 ms)
    chprintf(DEBUG_SD, "drogue line is alive\r\n");
    msg_t drogue_line = palWaitLineTimeout(LINE_ISO_DROGUE, TIME_INFINITE);

    if (drogue_line == MSG_OK) {
      chprintf(DEBUG_SD, "Drogue Signal: High\r\n");
    }
    // don't care for these, just want to check if any signal is returned
    if (drogue_line == MSG_RESET) {
      chprintf(DEBUG_SD, "Drogue Signal: Reset\r\n");
    }
    if (drogue_line == MSG_TIMEOUT) {
      chprintf(DEBUG_SD, "Drogue Signal: TIMEOUT\r\n");
    }
  }
}
