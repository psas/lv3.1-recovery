/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

// ChibiOS includes

#include <string.h>
#include "beep.h"
#include "blinky.h"
#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "mainchute.h"
#include "position.h"
#include "recovery.h"
#include "shell.h"
#include "telemetrum.h"

volatile enum recoverystatetype recoveryState = disarmed;
volatile enum PositionCommandtype PositionCommand = idle;
volatile enum mainchutecommandtype mainchuteCommand = idle_m;

//===========================================================================================
// Shell stuff
//===========================================================================================

static THD_WORKING_AREA(waShell, 1024);

void print_hall_sensors(void) {
  extern position_state_t position_state;
  chprintf(DEBUG_SD, "\r\nSensor 1: %d %s\r\n", position_state.sensor1,
           sensor_state_t_to_str(position_state.sensor1_state));
  chprintf(DEBUG_SD, "Sensor 2: %d %s\r\n", position_state.sensor2,
           sensor_state_t_to_str(position_state.sensor2_state));
  chprintf(DEBUG_SD, "Ring Position: %s\r\n",
           ring_position_t_to_str(position_state.ring_position));
}

static void cmd_state(BaseSequentialStream* chp, int argc, char* argv[]) {
  print_hall_sensors();

  /**/
  if (argc < 1) {
    switch (recoveryState) {
      case armed:
        chprintf(chp,
                 "State = ARMED (valid commands: 'arm', 'disarm', and "
                 "'reset').\r\n");
        break;

      case disarmed:
        chprintf(chp,
                 "State = DISARMED (valid commands: 'arm', 'disarm' and "
                 "'reset')\r\n");
        break; /* optional */

      default: /* Optional */
        chprintf(chp, "STATE = INVALID; SWITCHING TO ARMED\r\n");
        recoveryState = armed;
    }
    chThdSleepMilliseconds(10);  // Wait for printout (100 char ~ 10 ms)
    return;
  }

  if (!strcmp(argv[0], "arm")) {
    recoveryState = armed;
    chprintf(chp, "STATE = ARMED\r\n");
  } else if (!strcmp(argv[0], "disarm")) {
    recoveryState = disarmed;
    chprintf(chp, "STATE = DISARMED\r\n");
  } else if (!strcmp(argv[0], "reset")) {
    NVIC_SystemReset();
  } else {
    chprintf(chp,
             "Usage: state <command>\r\n"
             "    arm, disarm, and reset\r\n"
             "\r\n");
  }
  chThdSleepMilliseconds(10);  // Wait for printout (100 char ~ 10 ms)
}

// TODO implement one function to drive the motor that takes arguments of
// checking sensors or not, and current values drivemotor l arguments
// [s(safe;default):f(force), c(current; int <  1750)] u arguments
// [s(safe;default):f(force), c(current; int <  1750)]

static void cmd_lock(BaseSequentialStream* chp, int argc, char* argv[]) {
  drive_motor(true, 10, true, 1000);
}

static void cmd_unlock(BaseSequentialStream* chp, int argc, char* argv[]) {
  drive_motor(false, 10, true, 1000);
}

static void cmd_stream(BaseSequentialStream* chp, int argc, char* argv[]) {
  uint8_t buf;
  do {
    print_hall_sensors();
    chThdSleepMilliseconds(1000);
  } while (!sdReadTimeout((SerialDriver*)chp, &buf, sizeof(buf), 0));
}

static void cmd_position(BaseSequentialStream* chp, int argc, char* argv[]) {
  if (argc == 1) {
    if (!strcmp(argv[0], "lock")) {
      chprintf(chp, "Command: LOCKING ERS \r\n");
      PositionCommand = lock;
      return;
    }
    if (!strcmp(argv[0], "unlock")) {
      chprintf(chp, "Command: UNLOCKING ERS \r\n");
      PositionCommand = unlock;
      return;
    }
  }
}

static void cmd_beep(BaseSequentialStream* chp, int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  chprintf(chp, "slayyyyy\r\n");
  beep();
}

static void cmd_help(BaseSequentialStream* chp, int argc, char* argv[]) {
  chprintf(chp,
           "state - print sensor state\r\n"
           "pos - print ring position\r\n"
           "u - unlock ring\r\n"
           "l - lock ring\r\n"
           "stream - continously print sensor state\r\n"
           "beep - beep\r\n"
           "help - print this message\r\n");
}

static const ShellCommand commands[] = {
    {"state", cmd_state},
    {"pos", cmd_position},
    {"u", cmd_unlock},  // dev debug use only
    {"l", cmd_lock},    // dev debug use only
    {"stream", cmd_stream},
    {"beep", cmd_beep},
    {"help?", cmd_help},

    {NULL, NULL}};

static const ShellConfig shell_cfg = {DEBUG_SD, commands};

//===========================================================================================
// main
//===========================================================================================

int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  // Activate the serial driver 2 using the driver default configuration.
  sdStart(&SD2, NULL);

  // Print a nice message that we're alive and don't let the shell stomp on the
  // message
  chprintf(DEBUG_SD, "\r\nPSAS ERS control board starting up\r\n");
  chThdSleepMilliseconds(10);  // Wait for printout (100 char ~ 10 ms)

  palSetLine(LINE_BATTREAD);  // BATT_READ (shore power)

  beep_init();
  beep();

  // START THEM THREADS
  chThdCreateStatic(waBlinkyThread, sizeof(waBlinkyThread), NORMALPRIO,
                    BlinkyThread, NULL);

  chThdCreateStatic(waTelemetrumThread, sizeof(waTelemetrumThread), NORMALPRIO,
                    TelemetrumThread, NULL);
  // chThdCreateStatic(waDrogueThread, sizeof(waDrogueThread), NORMALPRIO,
  // DrogueThread, NULL);
  chThdCreateStatic(waMainchuteThread, sizeof(waMainchuteThread), NORMALPRIO,
                    MainchuteThread, NULL);
  chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread,
                    (void*)&shell_cfg);
  chThdCreateStatic(waPositionThread, sizeof(waPositionThread), NORMALPRIO,
                    PositionThread, NULL);

  while (true) {
    chThdSleepMilliseconds(500);
  }
}
