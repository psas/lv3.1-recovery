/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
// #include <zephyr/sys/printk.h>
#include <inttypes.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gpio_in, LOG_LEVEL_INF);

#include <keeper.h>

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_GPIO_THREAD_SLEEP_MS 12000

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec iso_drogue = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

#define SW1_NODE DT_ALIAS(sw1)
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec iso_main = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios, {0});

#define SW2_NODE DT_ALIAS(sw2)
#if !DT_NODE_HAS_STATUS(SW2_NODE, okay)
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec not_umb_on = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, {0});

#define SW3_NODE DT_ALIAS(sw3)
#if !DT_NODE_HAS_STATUS(SW3_NODE, okay)
#error "Unsupported board: sw3 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec not_motor_faila = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios, {0});

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

// TODO [ ] Figure out how best to utilize GPIO event driven interrupts,
//   since we'll be starting out with a thread to periodically read GPIOs
//   Maybe we can do away with such a thread, or have it at least not poll?
//   Going to try both ways to see what is most simple sufficient solution.

static struct gpio_callback pin_iso_drogue_cb_data;
static struct gpio_callback pin_iso_main_cb_data;
static struct gpio_callback pin_not_umb_on_cb_data;
static struct gpio_callback pin_not_motor_faila_cb_data;

#define GPIO_IN_THREAD_STACK_SIZE 512
#define GPIO_IN_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(gpio_in_thread_stack, GPIO_IN_THREAD_STACK_SIZE);

struct k_thread gpio_in_thread_data;

enum ers_input_signals
{
	ERS_SIG_ISO_DROGUE,
	ERS_SIG_ISO_MAIN,
	ERS_SIG_NOT_UMB_ON,
	ERS_SIG_NOT_MOTOR_FAILA,
	ERS_NUM_GPIO_INPUTS
};

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t gpio_in_configure_iso_drogue(void)
{
        if (!gpio_is_ready_dt(&iso_drogue)) {
                printk("Error: iso_drogue device %s is not ready\n",
                       iso_drogue.port->name);
                return -EIO;
        }

        int32_t rc = gpio_pin_configure_dt(&iso_drogue, GPIO_INPUT);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, iso_drogue.port->name, iso_drogue.pin);
                return -EINVAL;
        }

// TODO [ ] Review with hardware experts whether "EDGE TO ACTIVE" is the
//  best or correct condition to trigger an interrupt:
        rc = gpio_pin_interrupt_configure_dt(&iso_drogue,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (rc != 0) {
                printk("Error %d: failed to configure interrupt on %s pin %d\n",
                        rc, iso_drogue.port->name, iso_drogue.pin);
                return -EINVAL;
        }

        return rc;
}

int32_t gpio_in_configure_iso_main(void)
{
        if (!gpio_is_ready_dt(&iso_main)) {
                printk("Error: iso_main device %s is not ready\n",
                       iso_main.port->name);
                return -EIO;
        }

        int32_t rc = gpio_pin_configure_dt(&iso_main, GPIO_INPUT);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, iso_main.port->name, iso_main.pin);
                return -EINVAL;
        }

// TODO [ ] Review with hardware experts whether "EDGE TO ACTIVE" is the
//  best or correct condition to trigger an interrupt:
        rc = gpio_pin_interrupt_configure_dt(&iso_main,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (rc != 0) {
                printk("Error %d: failed to configure interrupt on %s pin %d\n",
                        rc, iso_main.port->name, iso_main.pin);
                return -EINVAL;
        }

        return rc;
}

int32_t gpio_in_configure_not_umb_on(void)
{
        if (!gpio_is_ready_dt(&not_umb_on)) {
                printk("Error: not_umb_on device %s is not ready\n",
                       not_umb_on.port->name);
                return -EIO;
        }

        int32_t rc = gpio_pin_configure_dt(&not_umb_on, GPIO_INPUT);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, not_umb_on.port->name, not_umb_on.pin);
                return -EINVAL;
        }

// TODO [ ] Review with hardware experts whether "EDGE TO ACTIVE" is the
//  best or correct condition to trigger an interrupt:
        rc = gpio_pin_interrupt_configure_dt(&not_umb_on,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (rc != 0) {
                printk("Error %d: failed to configure interrupt on %s pin %d\n",
                        rc, not_umb_on.port->name, not_umb_on.pin);
                return -EINVAL;
        }

        return rc;
}

int32_t gpio_in_configure_not_motor_faila(void)
{
        if (!gpio_is_ready_dt(&not_motor_faila)) {
                printk("Error: not_motor_faila device %s is not ready\n",
                       not_motor_faila.port->name);
                return -EIO;
        }

        int32_t rc = gpio_pin_configure_dt(&not_motor_faila, GPIO_INPUT);
        if (rc != 0) {
                printk("Error %d: failed to configure %s pin %d\n",
                       rc, not_motor_faila.port->name, not_motor_faila.pin);
                return -EINVAL;
        }

// TODO [ ] Review with hardware experts whether "EDGE TO ACTIVE" is the
//  best or correct condition to trigger an interrupt:
        rc = gpio_pin_interrupt_configure_dt(&not_motor_faila,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (rc != 0) {
                printk("Error %d: failed to configure interrupt on %s pin %d\n",
                        rc, not_motor_faila.port->name, not_motor_faila.pin);
                return -EINVAL;
        }

        return rc;
}

// GPIOs used as outputs

int32_t gpio_in_configure_deploy1(void)
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

int32_t gpio_in_configure_deploy2(void)
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

int32_t gpio_in_configure_not_motor_ps(void)
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

// TODO [ ] Add check for ERS GPIO module initialized.

int32_t ers_gpios_set_deploy1(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy1.port, deploy1.pin, value);
	return rc;
}

int32_t ers_gpios_set_deploy2(const uint32_t value)
{
	int32_t rc = gpio_pin_set(deploy2.port, deploy2.pin, value);
	return rc;
}

int32_t ers_gpios_set_not_motor_ps(const uint32_t value)
{
	int32_t rc = gpio_pin_set(not_motor_ps.port, not_motor_ps.pin, value);
	return rc;
}


void gpio_in_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	uint32_t val[ERS_NUM_GPIO_INPUTS] = {0};
	int32_t rc = 0;

	while (1)
	{
		// LOG_INF("gpio_in stub");

		val[ERS_SIG_ISO_DROGUE] = gpio_pin_get_dt(&iso_drogue);
		val[ERS_SIG_ISO_MAIN] = gpio_pin_get_dt(&iso_main);
		val[ERS_SIG_NOT_UMB_ON] = gpio_pin_get_dt(&not_umb_on);
		val[ERS_SIG_NOT_MOTOR_FAILA] = gpio_pin_get_dt(&not_motor_faila);

		ek_get_sys_diag_mode(&rc);
		if (rc > 0)
		{
			LOG_INF("drogue, main, umb, motor_fail: %d, %d, %d, %d",
				val[ERS_SIG_ISO_DROGUE],
				val[ERS_SIG_ISO_MAIN],
				val[ERS_SIG_NOT_UMB_ON],
				val[ERS_SIG_NOT_MOTOR_FAILA]
				);
		}

		k_msleep(ERS_GPIO_THREAD_SLEEP_MS);
	}
}

int32_t ers_init_gpio_in(void)
{
        int32_t rc = 0;

	rc = gpio_in_configure_iso_drogue();
	if (rc)
	{
		LOG_ERR("Failed to configure GPIO for iso_drogue signal in, err %d", rc);
		return rc;
	}

	rc = gpio_in_configure_iso_main();
	if (rc)
	{
		LOG_ERR("Failed to configure GPIO for iso_main signal in, err %d", rc);
		return rc;
	}

	rc = gpio_in_configure_not_umb_on();
	if (rc)
	{
		LOG_ERR("Failed to configure GPIO for not_umb_on signal in, err %d", rc);
		return rc;
	}

	rc = gpio_in_configure_not_motor_faila();
	if (rc)
	{
		LOG_ERR("Failed to configure GPIO for not_motor_faila signal in, err %d", rc);
		return rc;
	}

// ERS GPIOs used for output:

	rc = gpio_in_configure_deploy1();
	if (rc)
	{
		LOG_ERR("Configure deploy1 signal out, err %d", rc);
		return rc;
	}

	rc = gpio_in_configure_deploy2();
	if (rc)
	{
		LOG_ERR("Configure deploy2 signal out, err %d", rc);
		return rc;
	}

	rc = gpio_in_configure_not_motor_ps();
	if (rc)
	{
		LOG_ERR("Configure not_motor_ps signal out, err %d", rc);
		return rc;
	}

        k_tid_t gpio_in_tid = k_thread_create(&gpio_in_thread_data,
                                 gpio_in_thread_stack,
                                 K_THREAD_STACK_SIZEOF(gpio_in_thread_stack),
                                 gpio_in_thread_entry, NULL, NULL, NULL,
                                 GPIO_IN_THREAD_PRIORITY, 0, K_NO_WAIT);
        if (!gpio_in_tid) {
                LOG_ERR("ERROR spawning shell support thread\n");
        }

        return rc;
}
