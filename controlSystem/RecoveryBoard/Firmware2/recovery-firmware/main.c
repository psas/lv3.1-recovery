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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"

/*
 * Define the chprinf serial stream (to serial device 2 on UART2)
 */
#define DEBUG_SD  (BaseSequentialStream *) &SD2


//===========================================================================================
// Blinky thread
//===========================================================================================

static THD_WORKING_AREA(waThread1, 256);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinky");
  //chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
  
  while (true) {
    palClearLine(LINE_LED);
//    palClearLine(LINE_SPKR);
    chThdSleepMilliseconds(100);
    palSetLine(LINE_LED);
//    palSetLine(LINE_SPKR);
    chThdSleepMilliseconds(100);
  }
}


//===========================================================================================
// Shell stuff
//===========================================================================================

static THD_WORKING_AREA(waShell, 1024);

static void cmd_DdaaaAaavVVvveEEEE(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    chprintf(chp, "You have successfully whined to Dave to write this code... :)\r\n");
}

static const ShellCommand commands[] = {
    {"DdaaaAaavVVvveEEEE", cmd_DdaaaAaavVVvveEEEE},
    {NULL, NULL}
};

static const ShellConfig shell_cfg = {
  DEBUG_SD,
  commands
};


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

  // Print a nice message that we're alive and don't let the shell stomp on the message
  chprintf(DEBUG_SD, "\r\nPSAS ERS control board starting up\r\n");
  chThdSleepMilliseconds(100);
  
  // START THEM THREADS
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread, (void *)&shell_cfg);
  
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
