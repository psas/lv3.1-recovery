/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <arbiter.h>
#include <ers-adc.h>
#include <ers-can.h>
#include <ers-dac.h>
#include <gpio-in.h>
#include <keeper.h>
#include <motor-control.h>
#include <ers-pwm.h>
#include <settings-ers.h>
#include <shell-support.h>
#include <status-led.h>

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
	static uint32_t loop_count = 0;
	int32_t rc = 0;

        rc = ers_init_gpio_in();
	LOG_INF("GPIO input pin initialization returns %d", rc);

        rc = ers_init_motor_ctrl();
	LOG_INF("motor control module init returns %d", rc);

	rc = adc_init();
	LOG_INF("ADC init code returns %d", rc);

	rc = ers_init_dac();
	LOG_INF("DAC init code returns %d", rc);

// TODO [ ] Create thread for CAN init routine which entails a `while (1)`
//  construct.
	rc = ers_init_can();
	LOG_INF("ERS CAN module init returns %d", rc);

	ers_settings_init();
	LOG_INF("just back from ERS settings module init");
	k_msleep(500);

        rc = ers_init_shell_support();
	LOG_INF("ERS command initialization returns %d", rc);

	rc = ers_init_keeper();
	LOG_INF("ERS data \"keeper\" initialization returns %d", rc);

	rc = ers_init_arbiter();
	LOG_INF("ERS arbitration module init returns %d", rc);

	rc = pwm_init();
	LOG_INF("ERS PWM module init returns %d", rc);

	rc = status_led_init();
	LOG_INF("- DEV 0108 - routine to configure led0 returns %d", rc);

	LOG_INF("main() entering 'while (1)' loop . . .");

	while (1)
	{
		loop_count++;
		ek_get_sys_diag_mode(&rc);
		if (rc > 0)
		{
			LOG_INF("- MARK -");
			// Note appears that a = hall1, b = hall2, c = battery, d = motor_isense
			uint32_t a, b, c, d;
			ekget_batt_read(&a);
			ekget_motor_isense(&b);
			ekget_hall_1(&c);
			ekget_hall_2(&d);
			LOG_INF("batt, motor, hall1, hall2: %u, %u, %u, %u", a, b, c, d);
		}

		k_msleep(200);
		uint32_t b;
		ekget_motor_isense(&b);
		// LOG_INF("motor_isense ADC_IN9 = %u", b);

		rc = pwm_play_melody();
		// TODO [ ] . . . do something with rc.

		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
