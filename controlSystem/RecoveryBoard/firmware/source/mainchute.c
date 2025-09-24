#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

THD_WORKING_AREA(waMainchuteThread, 256);

THD_FUNCTION(MainchuteThread, arg) {
  (void)arg;
  chRegSetThreadName("Mainchute");
  msg_t main_line = palWaitLineTimeout(LINE_ISO_MAIN, TIME_INFINITE);
  if (main_line == MSG_OK) {
    chprintf(DEBUG_SD, "Main Signal: High");
  }
}
