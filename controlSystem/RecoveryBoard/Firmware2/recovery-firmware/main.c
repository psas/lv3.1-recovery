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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <string.h>
#include "shell.h"
#include "recovery.h"

// Project includes

#include "blinky.h"
#include "telemetrum.h"

//===========================================================================================
// Global variables because I'm OLDSKOOLCOOL
//===========================================================================================

volatile enum recoverystate recoveryState = armed;
volatile int fireDrogue = FALSE;
volatile int fireMain = FALSE;


//===========================================================================================
// Shell stuff
//===========================================================================================

static THD_WORKING_AREA(waShell, 1024);

static void cmd_DdaaaAaavVVvveEEEE(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    chprintf(chp, "You have successfully whined to Dave to write this code... :)\r\n");
}

static void cmd_state(BaseSequentialStream *chp, int argc, char *argv[]) {

    if (argc < 1) {
        switch(recoveryState) {
  
        case armed:
            chprintf(chp, "State = ARMED (valid commands: 'arm', 'disarm', and 'reset').\r\n");
            break;
  
        case disarmed:
            chprintf(chp, "State = DISARMED (valid commands: 'arm', 'disarm' and 'reset')\r\n");
            break; /* optional */
      
        default : /* Optional */
            chprintf(chp, "STATE = INVALID; SWITCHING TO ARMED\r\n");
            recoveryState = armed;
        }
        return;
    }
    
    if (!strcmp(argv[0], "arm")) {
        recoveryState = armed;
        chprintf(chp, "STATE = ARMED\r\n");
    } 
    else if (!strcmp(argv[0], "disarm")) {
        recoveryState = disarmed;
        chprintf(chp, "STATE = DISARMED\r\n");
    }
    else if (!strcmp(argv[0], "reset")) {
        NVIC_SystemReset();
    } 
    else {
        chprintf(chp,"Usage: state <command>\r\n"
                     "    arm, disarm, and reset\r\n"
                     "\r\n");
    }
}

static void cmd_fire(BaseSequentialStream *chp, int argc, char *argv[]) {

    if (argc == 1) {
        if (!strcmp(argv[0], "drogue")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "FIRING DROGUE (DC Motor)\r\n");
                fireDrogue = TRUE;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY FIRE DROGUE IF ARMED.\r\n"); 
            return;
        }
        if (!strcmp(argv[0], "main")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "FIRING MAIN (Linear actuator)\r\n");
                fireMain = TRUE;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY FIRE MAIN IF ARMED.\r\n");  
            return;
        }
    }

    chprintf(chp,"Usage: fire <type>\r\n"
                "    drogue, main\r\n"
                     "\r\n");
}

static const ShellCommand commands[] = {
    {"DdaaaAaavVVvveEEEE", cmd_DdaaaAaavVVvveEEEE},
    {"state", cmd_state},
    {"fire", cmd_fire},
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
    
    chThdCreateStatic(waBlinkyThread, sizeof(waBlinkyThread), NORMALPRIO, BlinkyThread, NULL);
    chThdCreateStatic(waTelemetrumThread, sizeof(waTelemetrumThread), NORMALPRIO, TelemetrumThread, NULL);
    chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread, (void *)&shell_cfg);
    
    while (true) {
        chThdSleepMilliseconds(500);
    }
}
