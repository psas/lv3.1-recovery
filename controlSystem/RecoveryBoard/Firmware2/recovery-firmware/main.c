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

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 256);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  chprintf(DEBUG_SD, "Blinker thread starting up (main.c)!\r\n");
  
  while (true) {
    palClearLine(LINE_LED);
//    palClearLine(LINE_SPKR);
    chThdSleepMilliseconds(100);
    palSetLine(LINE_LED);
//    palSetLine(LINE_SPKR);
    chThdSleepMilliseconds(100);
  }
}


void cmd_DdaaaAaavVVvveEEEE(BaseSequentialStream *chp, int argc, char *argv[]) {
	chprintf(chp, "You have successfully whined to Dave to write this code... :)\r\n");
}


static THD_WORKING_AREA(waShell, 2048);

static const ShellCommand commands[] = {
  {"DdaaaAaavVVvveEEEE", cmd_DdaaaAaavVVvveEEEE},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  DEBUG_SD,
  commands
};


/*
 * Application entry point.
 */
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

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

  /*
   * Create the blinker thread.
   */
  chprintf(DEBUG_SD, "\r\nPSAS ERS control board starting up (main.c)!\r\n");
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread, NULL);
  /*
   * main() thread activity
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
