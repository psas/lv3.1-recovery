/**
 * @file
 * @note ERS Zephyr firmware - status LED module
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(status_led, CONFIG_STATUS_LED_LOG_LEVEL);

#include "keeper.h"

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

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t configure_led(void)
{
	int32_t rc = 0;

        if (!gpio_is_ready_dt(&led)) {
                return -ENODEV;
        }

        rc = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	return rc;
}

void status_led_timer_handler(struct k_timer *dummy)
{
	uint32_t led_state = 0;
	int32_t rc = 0;

#if CONFIG_APP_CONTROLS_ERS_STATUS_LED
	ek_get_status_led_config(&led_state);
	if (!(led_state && STATUS_LED_ENABLE_BIT)) {
		rc = gpio_pin_set_dt(&led, 1);
		if (rc != 0) {
			LOG_ERR("Failed to turn on status LED, err %d", rc);
		}
	}
#else
	rc = gpio_pin_toggle_dt(&led);
	if (rc < 0) {
		LOG_ERR("Failed to toggle status LED, err %d", rc);
	}
	led_state = !led_state;
#endif
}

K_TIMER_DEFINE(status_led_timer, status_led_timer_handler, NULL);

int32_t status_led_init(void)
{
        int32_t rc = 0;
	if (configure_led() != 0)
	{
		LOG_ERR("Failed to init GPIO for status LED, err %d", rc);
		return rc;
	}

	// LOG_INF("- DEV 0214 - RETURNING EARLY . . .");
	// return rc;

	LOG_INF("- DEV 0214 - STARTING TIMER FOR STATUS LED . . .");
	k_timer_start(&status_led_timer, K_MSEC(LED_START_DURATION_MS), K_MSEC(LED_PERIOD_MS));
	return rc;
}
