/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pwm, LOG_LEVEL_INF);

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct pwm_dt_spec pwm_buzzer = PWM_DT_SPEC_GET(DT_ALIAS(buzzer));

#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

static uint32_t max_period_fs;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t pwm_init_alt_channel(void)
{
	LOG_INF("PWM set up for ERS buzzer\n");

	if (!pwm_is_ready_dt(&pwm_buzzer)) {
		LOG_ERR("Error: PWM device %s is not ready\n",
		       pwm_buzzer.dev->name);
		return -ENODEV;
	}

	return 0;
}

int32_t pwm_init(void)
{
	int32_t rc = 0;

	LOG_INF("initializing PWM for device '%s'\n", pwm_led0.dev->name);

	if (!pwm_is_ready_dt(&pwm_led0)) {
		LOG_ERR("Error: PWM device %s is not ready\n",
		       pwm_led0.dev->name);
		return -ENODEV;
	}

	/*
	 * In case the default MAX_PERIOD value cannot be set for
	 * some PWM hardware, decrease its value until it can.
	 *
	 * Keep its value at least MIN_PERIOD * 4 to make sure
	 * the sample changes frequency at least once.
	 */
	LOG_INF("Calibrating for channel %d...\n", pwm_led0.channel);
	max_period_fs = MAX_PERIOD;
	while (pwm_set_dt(&pwm_led0, max_period_fs, max_period_fs / 2U)) {
		max_period_fs /= 2U;
		if (max_period_fs < (4U * MIN_PERIOD)) {
			LOG_ERR("Error: PWM device "
			       "does not support a period at least %lu\n",
			       4U * MIN_PERIOD);
			return -ENOTSUP;
		}
	}

	LOG_INF("Done calibrating; maximum/minimum periods %u/%lu nsec\n",
	       max_period_fs, MIN_PERIOD);

	rc = pwm_init_alt_channel();
	return rc;
}

/*
Using period 15625000
Using period 7812500   . . . 256
Using period 15625000  . . . 128
Using period 31250000   . . . 64
Using period 62500000   . . . 32
Using period 125000000  . . . 16
Using period 250000000   . . . 8
Using period 500000000   . . . 4
Using period 1000000000  . . . 2 Hertz
Using period 500000000

From ChibiOS ERS work, source file beep.c:

 16   // Sweeps through the speaker frequency range once a second from low to high
 17   // by changing the PWM period. At a PWM timer rate of 100khz the periods
 18   // correspond to:
 19   // - 28 -> ~3570Hz
 20   // - 25 ->  4000Hz
 21   // - 23 -> ~4350Hz
*/

int32_t pwm_play_melody(void)
{
	uint32_t period;
	uint32_t note_count = 5;
	int ret;

// Use 500000000 to achieve 2Hz PWD signal on ERS orange LED
// (buzzer will require much higher frequency to produce audible sound)
#define DEV_STARTING_PERIOD 62500000
#define DEV_MAX_PERIOD 1000000000
	// period = max_period_fs;
	period = DEV_STARTING_PERIOD;
	while (note_count > 0) {
		ret = pwm_set_dt(&pwm_led0, period, period / 2U);
		if (ret) {
			LOG_ERR("Error %d: failed to set pulse width", ret);
			LOG_ERR("on device %s", pwm_led0.dev->name);
			return 0;
		}
#if 1
		ret = pwm_set_dt(&pwm_buzzer, period, period / 2U);
		if (ret) {
			LOG_ERR("Error %d: failed to set pulse width", ret);
			LOG_ERR("on device %s", pwm_buzzer.dev->name);
			return 0;
		}
#endif // 0
		LOG_INF("Using period %d\n", period);

		period = (period / 2U);
		note_count--;
		// k_sleep(K_SECONDS(1U));
		k_msleep(500);
	}
	return 0;
}
