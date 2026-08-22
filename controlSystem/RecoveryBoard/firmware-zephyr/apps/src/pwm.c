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

static const struct pwm_dt_spec pwm_buzzer = PWM_DT_SPEC_GET(DT_ALIAS(buzzer));

#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

static uint32_t max_period_fs;

struct k_thread pwm_thread_data;

K_THREAD_STACK_DEFINE(pwm_thread_stack, CONFIG_PWM_THREAD_STACK_SIZE);

#define PWM_MAX_HEARTBEAT_COUNT 5

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

static int32_t init_pwm_hardware(void)
{
	int32_t rc = 0;

	// LOG_INF("initializing PWM for device '%s'\n", pwm_buzzer.dev->name);

	if (!pwm_is_ready_dt(&pwm_buzzer)) {
		LOG_ERR("Error: PWM device %s is not ready\n",
		       pwm_buzzer.dev->name);
		return -ENODEV;
	}

	/*
	 * In case the default MAX_PERIOD value cannot be set for
	 * some PWM hardware, decrease its value until it can.
	 *
	 * Keep its value at least MIN_PERIOD * 4 to make sure
	 * the sample changes frequency at least once.
	 */
	// LOG_INF("Calibrating for channel %d...\n", pwm_buzzer.channel);
	max_period_fs = MAX_PERIOD;
	while (pwm_set_dt(&pwm_buzzer, max_period_fs, max_period_fs / 2U)) {
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

static int32_t buzzer_heartbeat(void)
{
	static uint32_t call_count = 0;

	if (call_count > PWM_MAX_HEARTBEAT_COUNT) {
		return 0;
	}
	call_count++;

	uint32_t period = 2800000;

	int32_t rc = pwm_set_dt(&pwm_buzzer, period, period / 2U);
	k_msleep(250);
	rc = pwm_set_dt(&pwm_buzzer, 0, 0);
	k_msleep(250);

	rc = pwm_set_dt(&pwm_buzzer, period, period / 2U);
	k_msleep(250);
	rc = pwm_set_dt(&pwm_buzzer, 0, 0);
	k_msleep(250);

	return rc;
}

int32_t pwm_play_melody(void)
{
	int rc = buzzer_heartbeat();
	if (rc != 0) {
		LOG_ERR("Failed to produce buzzer heartbeat audio, err %d", rc);
	}
	// k_sleep(K_SECONDS(2U));
	return 0;
}

void pwm_thread_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	int32_t rc = 0;

	while (1) {
		rc = pwm_play_melody();
		if (rc < 0) {
			LOG_ERR("Failed to play current audio note, err %d", rc);
		}
		// TODO [ ] Create symbol for PWM thread sleep period
		k_msleep(750);
	}
}

int32_t pwm_init(void)
{
	int32_t rc = init_pwm_hardware();
	if (rc < 0) {
		LOG_ERR("Failed to initialize PWM peripheral, err %d", rc);
		goto done;
	}

	k_tid_t pwm_tid = k_thread_create(&pwm_thread_data,
					pwm_thread_stack,
					K_THREAD_STACK_SIZEOF(pwm_thread_stack),
					pwm_thread_entry,
					NULL, NULL, NULL,
					CONFIG_PWM_THREAD_PRIORITY,
					0, K_NO_WAIT);

	k_thread_name_set(pwm_tid, "pwm_thread");

	if (!pwm_tid) {
		LOG_ERR("ERROR spawning PWM thread\n");
	}

done:
	return rc;
}
