/* PWM driven beep
 *
 * This drives the buzzer on the Recovery Board. The circuit is attached to pin
 * PB15. It expects a 50% duty cycle square wave optimally at 4000Hz but
 * supports frequencies between 3500Hz and 4500Hz. Pin PB15 can be configured
 * to Timer 1 Channel 3 complementary output (TIM1_CH3N) and operated as a PWM.
 */

#include "ch.h"
#include "hal.h"

static THD_WORKING_AREA(waBeepThread, 128);
static THD_FUNCTION(BeepThread, pwm) {
  chRegSetThreadName("Beep");

  // Sweeps through the speaker frequency range once a second from low to high
  // by changing the PWM period. At a PWM timer rate of 100khz the periods
  // correspond to:
  // - 28 -> ~3570Hz
  // - 25 ->  4000Hz
  // - 23 -> ~4350Hz
  const pwmcnt_t periods[] = {28,27,26,25,24,23};
  const int len = sizeof(periods)/sizeof(periods[0]);

  while (true) {
    for(int i = 0; i < len; i++) {
      pwmChangePeriod(pwm, periods[i]);
      chThdSleepMilliseconds(1000/len);
    }
  }
}

// Note that while the STM indexes PWM/Timer channels from 1 but ChibiOS
// indexes them from 0, so CHAN is one less.
#define PWMD &PWMD1
#define CHAN 2

static PWMConfig pwmcfg = {
  .frequency = 100000, // chosen arbitrarily, but high enough to make 4KHz easy
  .period = 25, // 4KHz at 100KHz PWM rate
  .channels = {
   {.mode = PWM_OUTPUT_DISABLED},
   {.mode = PWM_OUTPUT_DISABLED},
   // PWM Channel 3 in complementary output mode. This mode is only available
   // through the low level driver, not officially part of the normal high
   // level interface, and also only works with STM32_PWM_USE_ADVANCED enabled.
   {.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH},
   {.mode = PWM_OUTPUT_DISABLED},
  },
};

static virtual_timer_t timer;

void beep_init(void) {
  chVTObjectInit(&timer);
  palSetPadMode(GPIOB, GPIOB_SPEAKER, PAL_MODE_ALTERNATE(2));
  pwmStart(PWMD, &pwmcfg);
  chThdCreateStatic(waBeepThread, sizeof(waBeepThread), NORMALPRIO, BeepThread, PWMD);
}

static void beep_stop(void * pwm) {
  pwmDisableChannel(pwm, CHAN);
}

void beep(void) {
  pwmEnableChannel(PWMD, CHAN, PWM_PERCENTAGE_TO_WIDTH(PWMD, 5000));
  chVTSet(&timer, chTimeS2I(1), beep_stop, PWMD);
}

