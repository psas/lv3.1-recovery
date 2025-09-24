#include "telemetrum.h"
#include <ch.h>
#include <hal.h>
#include "chprintf.h"
#include "recovery.h"
// telemetrum signals low when active, circuit is then setup to drive iso
// signals high.
// the telemtrum sends the signal in a pulse of 50ms
// we want to be on the lookout for the two signals and execute them as soon as
// possible they NEED to be on the highest priority after ring position possibly
// possibly want to run them on different threads?
//
THD_WORKING_AREA(waTelemetrumThread, 256);

THD_FUNCTION(TelemetrumThread, arg) {

}


