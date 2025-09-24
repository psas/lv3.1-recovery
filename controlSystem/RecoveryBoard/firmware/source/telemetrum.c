#include "telemetrum.h"
#include <ch.h>
#include <hal.h>
#include "chprintf.h"
#include "recovery.h"

// telemetrum signals low when active, circuit is then setup to drive iso
// signals high.
// the telemtrum sends the signal in a pulse of 50ms
// we want to be on the lookout for the two signals and execute them as soon as possible 
// they NEED to be on the highest priority after ring position possibly
// possibly want to run them on different threads?
//
static void callback_drogue_high(void *arg) {
  (void)arg;
  chprintf(DEBUG_SD, "Drogue Signal: HIGH\r\n");
}

static void callback_main_high(void *arg) {
  (void)arg;
  chprintf(DEBUG_SD, "Main Signal: HIGH\r\n");
}


THD_WORKING_AREA(waTelemetrumThread, 256);

THD_FUNCTION(TelemetrumThread, arg) {
  (void)arg;
  palEnableLineEvent(LINE_ISO_DROGUE, PAL_EVENT_MODE_RISING_EDGE);

  palSetLineCallback(LINE_ISO_DROGUE, callback_drogue_high, NULL);

  palEnableLineEvent(LINE_ISO_MAIN, PAL_EVENT_MODE_RISING_EDGE);

  palSetLineCallback(LINE_ISO_MAIN, callback_main_high, NULL);

  palEnableLineEvent(LINE_ISO_DROGUE, PAL_EVENT_MODE_RISING_EDGE);
}

static void get_battery_status(void) {
  int batstatus = palReadLine(LINE_BATTREAD);
  if (batstatus == 1) {
    chprintf(DEBUG_SD, "Battery Signal: HIGH\r\n\n");
  }
  if (batstatus == 0) {
    chprintf(DEBUG_SD, "Battery Signal: LOW\r\n\n");
  }
}
