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
#include <shell-support.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_MAIN_LOOP_PERIOD_MS 1000

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

        rc = ers_init_shell_support();
	LOG_INF("ERS command initialization returns %d", rc);

	rc = ers_init_keeper();
	LOG_INF("ERS data \"keeper\" initialization returns %d", rc);

	rc = ers_init_arbiter();
	LOG_INF("ERS arbitration module init returns %d", rc);

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

		if (loop_count % 2)
		{
			rc = ers_set_dac_output(4080);
 			// LOG_INF("1015 - set DAC output call returns %d", rc);

			rc = motor_ctrl_drive_deploy1_high();
 			// LOG_INF("1015 - driving deploy1 high, deploy2 low returns %d", rc);
		}
		else
		{
			rc = ers_set_dac_output(2040);
 			// LOG_INF("1015 - set DAC output call returns %d", rc);

			rc = motor_ctrl_drive_deploy2_high();
 			// LOG_INF("1015 - driving deploy2 high, deploy1 low returns %d", rc);
		}

		rc = motor_ctrl_set_led0((loop_count / 2) & 0x1);
		rc = motor_ctrl_set_not_motor_ps((loop_count / 2) & 0x1);
		if (rc != 0)
		{
			// LOG_ERR("Failed to set not_motor_ps pin to %u, error %d",
			//  ((loop_count / 2) & 0x1), rc);
		}
		else
		{
			// LOG_INF("Set not_motor_ps pin to %d", ((loop_count / 2) & 0x1));
		}

		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
