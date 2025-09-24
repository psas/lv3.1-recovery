#include "telemetrum.h"
#include "ch.h"
#include "recovery.h"
#include "chprintf.h"
#include "hal.h"

THD_WORKING_AREA(waTelemetrumThread, 256);

THD_FUNCTION(TelemetrumThread, arg) {
  (void)arg;  
}

void get_telemetrum_line_status() {
  int droguestatus = palReadLine(LINE_ISO_DROGUE);
  int mainstatus = palReadLine(LINE_ISO_MAIN);
 // int rrstatus = palReadLine(LINE_ROCKETREADY);
  int batstatus = palReadLine(LINE_BATTREAD);
  if (droguestatus == 0) {
    chprintf(DEBUG_SD, "Drogue Signal: LOW\r\n");
  }
  if (droguestatus == 1) {
    chprintf(DEBUG_SD,"Drogue Signal: HIGH\r\n");
  } else {
    chprintf(DEBUG_SD,"Drogue Signal: UNDEF\r\n");
  }
  if (mainstatus == 0) {
    chprintf(DEBUG_SD,"Main Signal: LOW\r\n");
  }
  if (mainstatus == 1) {
    chprintf(DEBUG_SD,"Main Signal: HIGH\r\n");
  } else {
    chprintf(DEBUG_SD,"Main Signal: UNDEF\r\n");
  }
  if (batstatus == 0) {
    chprintf(DEBUG_SD,"Battery Signal: LOW\n");
  }
  if (batstatus == 1) {
    chprintf(DEBUG_SD,"Battery Signal: HIGH\n");
  } else {
    chprintf(DEBUG_SD,"Battery Signal: UNDEF\n");
  }
}
