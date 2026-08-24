/**
 * @file
 * @note ERS Zephyr firmware - status LED module
 */

#include "status-led.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(status_led, CONFIG_STATUS_LED_LOG_LEVEL);

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

#define LED_START_DURATION_MS 1000
#define LED_PERIOD_MS 500
#define STATUS_LED_ENABLE_BIT 0x01

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static bool flag_led_initialized = false;

static enum ers_status_led_pattern led_pattern_fs = STATUS_LED_HEARTBEAT;

#define STATUS_LED_GPIO_LEVEL_OFF 1

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

static int32_t configure_led(void)
{
	int32_t rc = 0;

        if (!gpio_is_ready_dt(&led)) {
                return -ENODEV;
        }

        rc = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	return rc;
}

static void status_led_timer_handler(struct k_timer *dummy)
{
	// enum ers_status_led_pattern pattern;
	int32_t rc = 0;

	// TODO [ ] Remove this pound if-else, and the second code block in it,
	//          in favor of a public API to toggle LED blinking.

#if 0
	if (!(led_state && STATUS_LED_ENABLE_BIT)) {
		rc = gpio_pin_set_dt(&led, 1);
		if (rc != 0) {
			LOG_ERR("Failed to turn on status LED, err %d", rc);
		}
	}
#endif

	if (led_pattern_fs == STATUS_LED_HEARTBEAT) {
		rc = gpio_pin_toggle_dt(&led);
		if (rc < 0) {
			LOG_ERR("Failed to toggle status LED, err %d", rc);
		}
		goto done;
	}

	if (led_pattern_fs == STATUS_LED_OFF) {
		rc = gpio_pin_set_dt(&led, STATUS_LED_GPIO_LEVEL_OFF);
		if (rc != 0) {
			LOG_ERR("Failed to turn off status LED, err %d", rc);
		}
	}
done:
	return;
}

K_TIMER_DEFINE(status_led_timer, status_led_timer_handler, NULL);

int32_t status_led_set_pattern(enum ers_status_led_pattern pattern)
{
// TODO [ ] Add mutex to this API

	if (!flag_led_initialized) {
		return -EIO;
	}

	// TODO [ ] Bounds check 'pattern' to fall within LED patterns enumeration:
	led_pattern_fs = pattern;

	return 0;
}

int32_t status_led_init(void)
{
	int32_t rc = configure_led();
	if (rc != 0)
	{
		LOG_ERR("Failed to init GPIO for status LED, err %d", rc);
		return rc;
	}

	k_timer_start(&status_led_timer, K_MSEC(LED_START_DURATION_MS), K_MSEC(LED_PERIOD_MS));
	flag_led_initialized = true;
	return rc;
}
