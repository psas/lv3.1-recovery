/*
    NeaPolis Innovation Summer Campus 2021 Examples
    Copyright (C) 2021 Ciro Mazzocchi
    [ciro.mazzocchi@outlook.com]

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

/*
 * Demo Test for PWM Driver.
 * PWM Signal, generated on PB4 pin.
 */

#include "ch.h"
#include "hal.h"

#include "stdio.h"

/*
 * PWM Driver Configuration.
 */
static void pwmPeriodCb(PWMDriver *pwmp){
  (void)pwmp;

  // This sets to HIGH the pin of green led
  palSetPad(GPIOB, GPIOB_SPEAKER);
};

static void pwmPulseCb(PWMDriver *pwmp){
  (void)pwmp;

  // This sets to LOW the pin of green led
  palClearPad(GPIOB, GPIOB_SPEAKER);
};

static PWMConfig pwmcfg = {
  10000,
  200,
  pwmPeriodCb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, pwmPulseCb},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};

/*
 * Duty cycle Beep Thread.
 */
static THD_WORKING_AREA(waBeepThread, 128);
static THD_FUNCTION(BeepThread, arg) {
  (void)arg;
  chRegSetThreadName("Beep");

  // This would define out sequence
  int delay[] = {250,750,1200,750};

  while (true) {
    // For each element of our sequence
    for(int i = 0; i <4; i++) {

      // This enables 0 channel with a new duty cycle
      pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, delay[i]));

      // This waits 1 second
      chThdSleepMilliseconds(1000);
    }
  }
}

int main(void) {
  halInit();
  chSysInit();

  // It configures PWM related PIN.
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2));

  // It stars PWM driver.
  pwmStart(&PWMD3, &pwmcfg);

  // It enables the periodic callback at the beginning of period
  pwmEnablePeriodicNotification(&PWMD3);

  // It enables the periodic callback at the end of pulse
  pwmEnableChannelNotification(&PWMD3,0);

  // It creates the Beep's thread
  chThdCreateStatic(waBeepThread, sizeof(waBeepThread), NORMALPRIO+2, BeepThread, NULL);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}