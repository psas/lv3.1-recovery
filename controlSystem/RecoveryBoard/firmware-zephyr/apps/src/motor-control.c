/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ERS board firmware source file motor-control.c
 */

#include "arbiter.h"
#include "dac-ers.h"
#include "keeper.h"
#include "settings-ers.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(ers_motor_ctrl, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define DOUT1_NODE DT_ALIAS(dout1)
#if !DT_NODE_HAS_STATUS(DOUT1_NODE, okay)
#error "Unsupported board: 'dout1' devicetree alias is not defined"
#endif
static const struct gpio_dt_spec deploy1 = GPIO_DT_SPEC_GET_OR(DOUT1_NODE, gpios, {0});

#define DOUT2_NODE DT_ALIAS(dout2)
#if !DT_NODE_HAS_STATUS(DOUT2_NODE, okay)
#error "Unsupported board: 'dout2' devicetree alias is not defined"
#endif
static const struct gpio_dt_spec deploy2 = GPIO_DT_SPEC_GET_OR(DOUT2_NODE, gpios, {0});

#define DOUT3_NODE DT_ALIAS(dout3)
#if !DT_NODE_HAS_STATUS(DOUT3_NODE, okay)
#error "Unsupported board: 'dout3' devicetree alias is not defined"
#endif
static const struct gpio_dt_spec not_motor_ps = GPIO_DT_SPEC_GET_OR(DOUT3_NODE, gpios, {0});

// TODO [ ] Add check for motor control module initialized.
// Flag to indiciate that this module is initialized:
static bool motor_control_initialized_fs = false;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

static int32_t mc_write_deploy1(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy1.port, deploy1.pin, value);
	return rc;
}

static int32_t mc_write_deploy2(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy2.port, deploy2.pin, value);
	return rc;
}

static int32_t mc_write_not_motor_ps(const uint32_t value)
{
	int32_t rc = gpio_pin_set(not_motor_ps.port, not_motor_ps.pin, value);
	return rc;
}

static int32_t mc_configure_deploy1(void)
{
        if (!gpio_is_ready_dt(&deploy1)) {
                LOG_ERR("Error: deploy1 device %s is not ready",
                       deploy1.port->name);
                return -EIO;
        }

	// Configure GPIO as output and initialize output state to high:
        int32_t rc = gpio_pin_configure_dt(&deploy1, GPIO_OUTPUT_HIGH);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, deploy1.port->name, deploy1.pin);
                return -EINVAL;
        }

	return rc;
}

static int32_t mc_configure_deploy2(void)
{
        if (!gpio_is_ready_dt(&deploy2)) {
                LOG_ERR("Error: deploy2 device %s is not ready",
                       deploy2.port->name);
                return -EIO;
        }

	// Configure GPIO as output and initialize output state to high:
        int32_t rc = gpio_pin_configure_dt(&deploy2, GPIO_OUTPUT_HIGH);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, deploy2.port->name, deploy2.pin);
                return -EINVAL;
        }

	return rc;
}

static int32_t mc_configure_not_motor_ps(void)
{
        if (!gpio_is_ready_dt(&not_motor_ps)) {
                LOG_ERR("Error: not_motor_ps device %s is not ready",
                       not_motor_ps.port->name);
                return -EIO;
        }

	// Configure GPIO as output and initialize output state to high:
        int32_t rc = gpio_pin_configure_dt(&not_motor_ps, GPIO_OUTPUT_HIGH);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, not_motor_ps.port->name, not_motor_ps.pin);
                return -EINVAL;
        }

	return rc;
}

static int32_t drive_to_lock(void)
{
	int32_t rc1, rc2;
	rc1 = mc_write_deploy1(1);
	rc2 = mc_write_deploy2(0);
	if ((rc1 == 0) && (rc2 == 0)) {
		return 0;
	} else {
		return -EINVAL;
	}
}

static int32_t drive_to_unlock(void)
{
	int32_t rc1, rc2;
	rc1 = mc_write_deploy1(0);
	rc2 = mc_write_deploy2(1);
	if ((rc1 == 0) && (rc2 == 0)) {
		return 0;
	} else {
		return -EINVAL;
	}
}

static uint32_t motor_current_in_adc_fs[CONFIG_MC_COUNT_RING_CHECKS] = {0};

// Some local formatting symbols to better show motor current readings
// from lock ring and unlock ring ops:
#define READING_WIDTH 6
#define READINGS_PER_LINE 8
#define MARGIN 8
#define LINE_LEN ((READING_WIDTH * READINGS_PER_LINE) + MARGIN)

static void show_motor_currents(void)
{
	char lbuf[LINE_LEN] = { 0 };
	uint32_t i = 0;
	uint32_t j = 1;
	uint32_t buf_len = 0;

	while (i < CONFIG_MC_COUNT_RING_CHECKS) {
		if ((i % READINGS_PER_LINE) != 0) {
			buf_len = strlen(lbuf);

			if (buf_len == 0) {
				snprintf(&lbuf[buf_len], (LINE_LEN - buf_len), "(%u)", j);
				buf_len = strlen(lbuf);
				j++;
			}

			snprintf(&lbuf[buf_len], (LINE_LEN - buf_len), " %u,",
			  motor_current_in_adc_fs[i]);
		} else {
			LOG_INF("%s", lbuf);
			memset(lbuf, 0, sizeof(lbuf));
			k_msleep(100);
		}
		i++;
	}

	LOG_INF("%s", lbuf);
	k_msleep(5);
}

static int32_t mc_update_lock_count(void)
{
	uint32_t val = 0;
	int32_t rc = 0;

	// Read lock count and increment:
	keeper_get_lock_event_count(&val);
	val += 1;

	// Write the updated lock count to run time data store:
	keeper_set_lock_event_count(val);
	LOG_INF("Keeper holds a ring lock count of %u", val);

	// Store the updated lock count to non-volatile memory:
	rc = settings_ers_store_value(KEY_NAME_LOCK_COUNT, (const void *)val, sizeof(val));
	if (rc != 0) {
		LOG_ERR("Failed to store count of ring lock events, err %d", rc);
	}

	return rc;
}

static int32_t mc_update_unlock_count(void)
{
	uint32_t val = 0;
	int32_t rc = 0;

	// Read unlock count and increment:
	keeper_get_unlock_event_count(&val);
	LOG_INF("Keeper holds a ring unlock count of %u", val);
	val += 1;

	// Write the updated unlock count to run time data store:
	keeper_set_unlock_event_count(val);

	// Store the updated unlock count to non-volatile memory:
	rc = settings_ers_store_value(KEY_NAME_UNLOCK_COUNT, (const void *)val, sizeof(val));
	if (rc != 0) {
		LOG_ERR("Failed to store count of ring unlock events, err %d", rc);
	}

	return rc;
}


int32_t mc_lock_ring(void)
{
// Set DAC output to create ~100m at H-bridge output
// Drive NOT_MOTOR_PS high to enable H-bridge
// Drive deploy signals to move ring to locked position
// Periodically check for ring position in locked state and in unknown state,
// turn off motor in those states.

	int32_t rc = 0;

	LOG_INF("M1 - DEPLOY1 high");
	// (1) make sure BDS63150 is on, not in power saving mode:
	rc = mc_write_not_motor_ps(0x0);
	if (rc != 0) {
	       	LOG_ERR("Failed to drive BDS63150 power mode pin, err %d", rc);
		goto enter_power_saving_mode;
       	}

	// (2) set DAC to produce minimal current needed to turn over lock ring motor:
	rc = dac_write_output_reg(CONFIG_MC_DAC_OUTPUT_FOR_CURRENT_LIMIT);
	if (rc != 0) {
	       	LOG_ERR("Failed to set DAC output level, err %d", rc);
		goto set_low_current_limit;
       	}

	// (3) apply logic levels to BDS63150 IN1, IN2 pins for H-bridge output:
	rc = drive_to_unlock();
	if (rc != 0) {
	       	LOG_ERR("Failed to drive BDS63150 DEPLOY 1 and or 2 lines, err %d", rc);
		goto set_low_current_limit;
       	}

	enum lock_ring_position ring_pos = RING_POS_UNKNOWN;
	uint32_t i;

	// Note:  the body of this FOR loop only reads values from the app's
	//  keeper module.  It does not write them.  Another module, the ADC
	//  module is periodically writing values to the keeper.  There is a
	//  non-obvious dependency here -- an implementation worth revisiting
	//  and possibly changing -- to assure that RING_CHECK_INTERVAL_MS is
	//  at least as long as the ADC "read all channels" interval.
	for (i = 0; i < CONFIG_MC_COUNT_RING_CHECKS; i++)
	{
		keeper_get_ring_position(&ring_pos);

		if ((ring_pos == RING_POS_LOCKED) ||
		    (ring_pos == RING_POS_LOCKED_FULLY_QUALIFIED))
		{
			break;
		}

		keeper_get_motor_isense(&motor_current_in_adc_fs[i]);
		k_msleep(CONFIG_MC_RING_CHECK_INTERVAL_MS);
	}
	LOG_INF("Stopping motor on ring position = %d", ring_pos);
	LOG_INF("Stopped motor after %u ring position checks", i);

set_low_current_limit:
	// (4) reduce current to motor to way low:
	LOG_INF("M1 - DAC output low . . .");
	rc = dac_write_output_reg(5);
	if (rc != 0) { LOG_ERR("Trouble set DAC out to near zero!"); }

	rc = mc_update_lock_count();

enter_power_saving_mode:
	// (5) set BDS63150 to power saving mode:
	rc = mc_write_not_motor_ps(0x1);
	if (rc != 0) { LOG_ERR("Trouble motor_ps!"); }

	LOG_INF("motor currents:");
	show_motor_currents();

done:
	return rc;
}

int32_t mc_unlock_ring(void)
{
	int32_t rc = 0;

	LOG_INF("M1 - DEPLOY1 high");
	// (1) make sure BDS63150 is on, not in power saving mode:
	rc = mc_write_not_motor_ps(0x0);
	if (rc != 0) {
		LOG_ERR("Trouble setting not_motor_ps low, err %d", rc);
		goto enter_power_saving_mode;
	}

	// (2) set DAC to produce minimal current needed to turn over lock ring motor:
	rc = dac_write_output_reg(CONFIG_MC_DAC_OUTPUT_FOR_CURRENT_LIMIT);
	if (rc != 0) {
		LOG_ERR("Trouble setting DAC out, err %d", rc);
		goto set_low_current_limit;
	}

	// (3) apply logic levels to BDS63150 IN1, IN2 pins for H-bridge output:
	rc = drive_to_lock();
	if (rc != 0) {
		LOG_ERR("Trouble driving DEPLOY1 high, err %d", rc);
		goto set_low_current_limit;
	}

	enum lock_ring_position ring_pos = RING_POS_UNKNOWN;
	uint32_t i;

	for (i = 0; i < CONFIG_MC_COUNT_RING_CHECKS; i++)
	{
		keeper_get_ring_position(&ring_pos);

		if ((ring_pos == RING_POS_UNLOCKED) ||
		    (ring_pos == RING_POS_UNLOCKED_FULLY_QUALIFIED))
		{
			LOG_INF("Stopping motor on ring position = %d", ring_pos);
			break;
		}
		k_msleep(CONFIG_MC_RING_CHECK_INTERVAL_MS);
	}

	LOG_INF("Stopping motor after %u ring position checks", i);

set_low_current_limit:
	// (4) reduce current to motor to very low:
	LOG_INF("M1 - DAC output low . . .");
	rc = dac_write_output_reg(5);
	if (rc != 0) {
		LOG_ERR("Trouble set DAC out to near zero!");
	}

	rc = mc_update_unlock_count();

enter_power_saving_mode:
	// (5) set BDS63150 to power saving mode:
	rc = mc_write_not_motor_ps(0x1);
	if (rc != 0) {
		LOG_ERR("Trouble motor_ps!");
	}

	return rc;
}

//----------------------------------------------------------------------
// - SECTION - initialize motor control module
//----------------------------------------------------------------------

int32_t ers_init_motor_ctrl(void)
{
        uint32_t event_count = 0;
        int32_t rc = 0;

// ERS GPIOs used for output:

	rc = mc_configure_deploy1();
	if (rc) {
		LOG_ERR("Configure deploy1 signal out, err %d", rc);
		goto done;
	}

	rc = mc_configure_deploy2();
	if (rc) {
		LOG_ERR("Configure deploy2 signal out, err %d", rc);
		goto done;
	}

	rc = mc_configure_not_motor_ps();
	if (rc) {
		LOG_ERR("Configure not_motor_ps signal out, err %d", rc);
		goto done;
	}

	// Drive NOT_MOTOR_PS high to assure motor H-bridge is powered:
	rc = mc_write_not_motor_ps(0);
	if (rc < 0) {
		LOG_ERR("Failed to drive not_motor_ps line, err %d", rc);
		goto done;
	}

	// Retrieve ring lock event count:
	rc = settings_ers_retrieve_value(KEY_NAME_LOCK_COUNT,
			&event_count, sizeof(event_count));
	if (rc < 0) {
		LOG_ERR("Failed to retrieve lock ring event count, err %d", rc);
		LOG_WRN("Resetting lock event count to zero . . .");
		event_count = 0;
	}

	// Write lock event count to run time data store:
	keeper_set_lock_event_count(event_count);

	// Retrieve ring unlock event count:
	rc = settings_ers_retrieve_value(KEY_NAME_UNLOCK_COUNT,
			&event_count, sizeof(event_count));
	if (rc < 0) {
		LOG_ERR("Failed to retrieve unlock ring event count, err %d", rc);
		LOG_WRN("Resetting unlock event count to zero . . .");
		event_count = 0;
	}

	// Write unlock event count to run time data store:
	keeper_set_unlock_event_count(event_count);

	motor_control_initialized_fs = true;

done:
        return rc; 
}
