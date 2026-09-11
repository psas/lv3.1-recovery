/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "adc-ers.h"
#include "arbiter.h"
#include "can-ers.h"
#include "dac-ers.h"
#include "pwm-ers.h"
#include "gpio-in.h"
#include "keeper.h"
#include "motor-control.h"
#include "settings-ers.h"
#include "status-led.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdio.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_MAIN_LOOP_PERIOD_MS 2000

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int main(void)
{
	int32_t rc = 0;

	// Setting up GPIOs is one of the first things to do for hardware safety:
	rc = gpio_in_init();
	LOG_INF("GPIO input pin initialization returns %d", rc);

	// The motor control module calls settings API, and is also important
	// for safe hardware operation:
	settings_ers_init();
	LOG_INF("just back from ERS settings module init");
	k_msleep(300);

	// ERS keeper module is used by motor and several other modules, initialize
	// this module before those who use it:
	rc = keeper_init();
	LOG_INF("ERS data \"keeper\" initialization returns %d", rc);

	// The motor module puts the H-brdige in a "motor off" state, so call this
	// module as early as possible:
        rc = ers_init_motor_ctrl();
	LOG_INF("motor control module init returns %d", rc);

	rc = adc_init();
	LOG_INF("ADC init code returns %d", rc);

	rc = dac_init();
	LOG_INF("DAC init code returns %d", rc);

	rc = ers_can_init();
	LOG_INF("ERS CAN module init returns %d", rc);

	rc = arbiter_init();
	LOG_INF("ERS arbitration module init returns %d", rc);

	rc = pwm_init();
	LOG_INF("ERS PWM module init returns %d", rc);

	rc = status_led_init();
	LOG_INF("routine to configure led0 returns %d", rc);

	rc = keeper_retrieve_hall_1_limits();
	if (rc != 0) {
		LOG_ERR("Failed to retrieve Hall sensor 1 cut-off values, err = %d", rc);
	} else {
		LOG_INF("Hall sensor 1 cut-off values retrieved");
	}

	rc = keeper_retrieve_hall_2_limits();
	if (rc != 0) {
		LOG_ERR("Failed to retrieve Hall sensor 2 cut-off values, err = %d", rc);
	} else {
		LOG_INF("Hall sensor 2 cut-off values retrieved");
	}

	// Play start up, or 'banner' audio pattern:
	rc = pwm_play_pattern(PWM_APP_15_NOTES);
	if (rc != 0) {
		LOG_ERR("Failed to play 15-note audio pattern, err = %d", rc);
	}

#if 0
	static uint32_t loop_count = 0;

	LOG_INF("main() entering 'while (1)' loop . . .");

	while (1)
	{
		loop_count++;
		keeper_get_diag_mode(&rc);
		// TODO [ ] Review call to check diagnostics mode and remove the
		//          following unconditional clearing of 'rc' when mode
		//          check is determined to be a sensible check:
		rc = 0; // - DEV 0226 -
		if (rc > 0)
		{
			LOG_INF("- MARK -");
			// Note appears that a = hall1, b = hall2, c = battery, d = motor_isense
			uint32_t a, b, c, d;
			keeper_get_batt_read(&a);
			keeper_get_motor_isense(&b);
			keeper_get_hall_1(&c);
			keeper_get_hall_2(&d);
			LOG_INF("batt, motor, hall1, hall2: %u, %u, %u, %u", a, b, c, d);
		}

		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
#else
	LOG_INF("- DEV 0909 - main exiting to test other app threads and timers");
	return 0;
#endif
}
