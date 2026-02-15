/**
 *  ERS Zephyr firmware - status LED module
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

#define STATUS_LED_ENABLE_BIT 0x00000001

// int32_t dev_toggle_led(void)
void status_led_timer_handler(struct k_timer *dummy)
{
	static bool led_state = true;
	uint32_t config = 0;
	int32_t rc = 0;

	ek_get_status_led_config(&config);
	if (!(config && STATUS_LED_ENABLE_BIT)) {
// TODO [ ] add second timer to monitor run-time status LED config, so that
//   this timer may turn itsef off and other modules may restart it. 
		rc = gpio_pin_set_dt(&led, 1);
		// TODO [ ] Check `rc` or annotate it as unused.
		return;
	}

	rc = gpio_pin_toggle_dt(&led);
	if (rc < 0) {
		// return rc;
	}
	led_state = !led_state;
	// return rc;
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
