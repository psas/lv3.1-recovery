/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ERS board firmware source file motor-control.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ers_motor_ctrl, LOG_LEVEL_INF);

#include <arbiter.h>
#include <ers-dac.h>
#include <keeper.h>

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

// - DEV 0928 BEGIN -
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
// - DEV 0928 END -

// - DEV 1019 BEGIN -
#define LED0_NODE DT_ALIAS(led0)
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: 'led0' devicetree alias is not defined"
#endif
// static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0});
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
// - DEV 1019 END -


// TODO [ ] Add check for motor control module initialized.

int32_t mc_set_deploy1(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy1.port, deploy1.pin, value);
	return rc;
}

int32_t mc_set_deploy2(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy2.port, deploy2.pin, value);
	return rc;
}

int32_t mc_set_not_motor_ps(const uint32_t value)
{
	int32_t rc = gpio_pin_set(not_motor_ps.port, not_motor_ps.pin, value);
	return rc;
}

int32_t mc_set_led0(const uint32_t value)
{
	int32_t rc = gpio_pin_set(led0.port, led0.pin, value);
	// LOG_INF("- 1019 - setting LED0 output pin to %u", value);
	return rc;
}

// GPIOs used as outputs

int32_t mc_configure_deploy1(void)
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

int32_t mc_configure_deploy2(void)
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

int32_t mc_configure_not_motor_ps(void)
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



int32_t mc_configure_led0(void)
{
        if (!gpio_is_ready_dt(&led0)) {
                LOG_ERR("Error: led0 device %s is not ready",
                       led0.port->name);
                return -EIO;
        }

	// Configure GPIO as output and initialize output state to high:
        int32_t rc = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, led0.port->name, led0.pin);
                return -EINVAL;
        }

	return rc;
}



int32_t mc_drive_deploy1_high(void)
{
	int32_t rc1, rc2;
	rc1 = mc_set_deploy1(1);
	rc2 = mc_set_deploy2(0);
	if ((rc1 == 0) && (rc2 == 0))
	{
		return 0;
	}
	else
	{
		return -EINVAL;
	}
}

int32_t mc_drive_deploy2_high(void)
{
	int32_t rc1, rc2;
	rc1 = mc_set_deploy1(0);
	rc2 = mc_set_deploy2(1);
	if ((rc1 == 0) && (rc2 == 0))
	{
		return 0;
	}
	else
	{
		return -EINVAL;
	}
}

#define DEV_DAC_SETTING_IN_SITU 800
#define RING_CHECK_INTERVAL_MS 10
#define COUNT_CHECKS 30

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
	rc = mc_set_not_motor_ps(0x0);
	if (rc != 0) { LOG_ERR("Trouble motor_ps!"); }

	// (2) set DAC to produce minimal current needed to turn over lock ring motor:
	rc = dac_set_output(DEV_DAC_SETTING_IN_SITU);
	if (rc != 0) { LOG_ERR("Trouble set DAC out!"); }

	// (3) apply logic levels to BDS63150 IN1, IN2 pins for H-bridge output:
	rc = mc_drive_deploy2_high();
	if (rc != 0) { LOG_ERR("Trouble set DEPLOY1, DEPLOY1!"); }

	enum lock_ring_position ring_pos = RING_POSITION_UNKNOWN;
	uint32_t i;

	for (i = 0; i < COUNT_CHECKS; i++)
	{
		get_detected_ring_position(&ring_pos);

		if ((ring_pos == RING_LOCKED) ||
		    (ring_pos == RING_LOCKED_FULLY_QUALIFIED))
		{
			LOG_INF("Stopping motor on ring position = %d", ring_pos);
			break;
		}
		k_msleep(RING_CHECK_INTERVAL_MS);
	}

	LOG_INF("Stopped motor after %u ring position checks", i);

	// (4) reduce current to motor to way low:
	LOG_INF("M1 - DAC output low . . .");
	rc = dac_set_output(5);
	if (rc != 0) { LOG_ERR("Trouble set DAC out to near zero!"); }

	// (5) set BDS63150 to power saving mode:
	rc = mc_set_not_motor_ps(0x1);
	if (rc != 0) { LOG_ERR("Trouble motor_ps!"); }

	return rc;
}

int32_t mc_unlock_ring(void)
{
	int32_t rc = 0;

	LOG_INF("M1 - DEPLOY1 high");
	// (1) make sure BDS63150 is on, not in power saving mode:
	rc = mc_set_not_motor_ps(0x0);
	if (rc != 0) { LOG_ERR("Trouble motor_ps!"); }

	// (2) set DAC to produce minimal current needed to turn over lock ring motor:
	rc = dac_set_output(DEV_DAC_SETTING_IN_SITU);
	if (rc != 0) { LOG_ERR("Trouble set DAC out!"); }

	// (3) apply logic levels to BDS63150 IN1, IN2 pins for H-bridge output:
	rc = mc_drive_deploy1_high();
	if (rc != 0) { LOG_ERR("Trouble set DEPLOY1, DEPLOY1!"); }

	enum lock_ring_position ring_pos = RING_POSITION_UNKNOWN;
	uint32_t i;

	for (i = 0; i < COUNT_CHECKS; i++)
	{
		get_detected_ring_position(&ring_pos);

		if ((ring_pos == RING_UNLOCKED) ||
		    (ring_pos == RING_UNLOCKED_FULLY_QUALIFIED))
		{
			LOG_INF("Stopping motor on ring position = %d", ring_pos);
			break;
		}
		k_msleep(RING_CHECK_INTERVAL_MS);
	}

	LOG_INF("Stopped motor after %u ring position checks", i);

	// (4) reduce current to motor to way low:
	LOG_INF("M1 - DAC output low . . .");
	rc = dac_set_output(5);
	if (rc != 0) { LOG_ERR("Trouble set DAC out to near zero!"); }

	// (5) set BDS63150 to power saving mode:
	rc = mc_set_not_motor_ps(0x1);
	if (rc != 0) { LOG_ERR("Trouble motor_ps!"); }

	return rc;
}

//----------------------------------------------------------------------
// - SECTION - initialize motor control module
//----------------------------------------------------------------------

int32_t ers_init_motor_ctrl(void)
{
        int32_t rc = 0;

// ERS GPIOs used for output:

	rc = mc_configure_deploy1();
	if (rc)
	{
		LOG_ERR("Configure deploy1 signal out, err %d", rc);
		return rc;
	}

	rc = mc_configure_deploy2();
	if (rc)
	{
		LOG_ERR("Configure deploy2 signal out, err %d", rc);
		return rc;
	}

	rc = mc_configure_not_motor_ps();
	if (rc)
	{
		LOG_ERR("Configure not_motor_ps signal out, err %d", rc);
		return rc;
	}


	// 1019
	rc = mc_configure_led0();
	LOG_ERR("- 1019 - Configure led0 signal out returns status %d", rc);
	// 1019


	// Drive NOT_MOTOR_PS high to assure motor H-bridge is powered:
	rc = mc_set_not_motor_ps(0);
	LOG_INF("- DEV 1015 - setting not_motor_ps to 1 returns %d", rc);

        return rc; 
}
