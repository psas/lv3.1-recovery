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
#include "drogue.h"
#include "mainchute.h"

//===========================================================================================
// Global variables because I'm OLDSKOOLCOOL
//===========================================================================================

volatile enum recoverystatetype recoveryState = armed;
volatile enum droguecommandtype drogueCommand = idle;
volatile enum mainchutecommandtype mainchuteCommand = idle_m;

//===========================================================================================
// Shell stuff
//===========================================================================================

static THD_WORKING_AREA(waShell, 1024);

static void cmd_DdaaaAaavVVvveEEEE(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    chprintf(chp, "You have successfully whined to Dave to write this code... :)\r\n");
    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
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
        chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
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
    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
}

static void cmd_drogue(BaseSequentialStream *chp, int argc, char *argv[]) {

    if (argc == 1) {
        if (!strcmp(argv[0], "fire")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: FIRING DROGUE (DC Motor)\r\n");
                drogueCommand = fire;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY FIRE DROGUE IF ARMED.\r\n"); 
            return;
        }
        if (!strcmp(argv[0], "lock")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: LOCKING DROGUE (DC Motor)\r\n");
                drogueCommand = lock;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY LOCK DROGUE IF ARMED.\r\n");  
            return;
        }
        if (!strcmp(argv[0], "unlock")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: UNLOCKING DROGUE (DC Motor)\r\n");
                drogueCommand = unlock;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY UNLOCK DROGUE IF ARMED.\r\n");  
            return;
        }
        if (!strcmp(argv[0], "stop")) {
            chprintf(chp, "STOPPING DROGUE (DC Motor)\r\n");
            drogueCommand = stop;
            return;
        }
        if (!strcmp(argv[0], "cw")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: PULSING CLOCKWISE (LOCKING) (DC Motor)\r\n");
                drogueCommand = cw;
            }
            else
                chprintf(chp, "INVALID: CAN'T PULSE DROGUE IF ARMED.\r\n");  
            return;
        }
        if (!strcmp(argv[0], "ccw")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: PULSING COUNTERCLOCKWISE (UNLOCKING) (DC Motor)\r\n");
                drogueCommand = ccw;
            }
            else
                chprintf(chp, "INVALID: CAN'T PULSE DROGUE IF ARMED.\r\n");  
            return;
        }
    }
   
    chprintf(chp,"Usage: d <command>\r\n"
                "    fire, lock, stop, cw, ccw\r\n"
                     "\r\n");
    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
}

static void cmd_mainchute(BaseSequentialStream *chp, int argc, char *argv[]) {

    if (argc == 1) {
        if (!strcmp(argv[0], "fire")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: FIRING MAIN CHUTE (LA)\r\n");
                mainchuteCommand = fire_m;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY FIRE MAIN IF ARMED.\r\n"); 
            return;
        }
        if (!strcmp(argv[0], "reset")) {
            if (recoveryState == disarmed) {
                chprintf(chp, "Command: RESETING MAIN CHUTE (LA)\r\n");
                mainchuteCommand = reset_m;
            }
            else
                chprintf(chp, "INVALID: CAN'T MANUALLY RESET MAIN IF ARMED.\r\n");  
            return;
        }
        if (!strcmp(argv[0], "stop")) {
            chprintf(chp, "Command: STOPPING MAIN CHUTE (LA)\r\n");
            mainchuteCommand = stop_m;
            return;
        }
    }
   
    chprintf(chp,"Usage: m <command>\r\n"
                "    fire, reset, stop\r\n"
                     "\r\n");
    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
}

static const ShellCommand commands[] = {
    {"DdaaaAaavVVvveEEEE", cmd_DdaaaAaavVVvveEEEE},
    {"state", cmd_state},
    {"d", cmd_drogue},
    {"m", cmd_mainchute},
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
    chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
    
    palSetLine(LINE_DCM_DIR);
    // START THEM THREADS

    chThdCreateStatic(waBlinkyThread, sizeof(waBlinkyThread), NORMALPRIO, BlinkyThread, NULL);
//    while (true) {
//          chThdSleepMilliseconds(500);
//      }
    chThdCreateStatic(waTelemetrumThread, sizeof(waTelemetrumThread), NORMALPRIO, TelemetrumThread, NULL);
    chThdCreateStatic(waDrogueThread, sizeof(waDrogueThread), NORMALPRIO, DrogueThread, NULL);
    chThdCreateStatic(waMainchuteThread, sizeof(waMainchuteThread), NORMALPRIO, MainchuteThread, NULL);    
    chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread, (void *)&shell_cfg);
    
    while (true) {
        chThdSleepMilliseconds(500);
    }
}
