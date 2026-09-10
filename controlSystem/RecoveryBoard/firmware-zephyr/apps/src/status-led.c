/**
 * @file
 * @note ERS Zephyr firmware - status LED module
 */

#include "ers-util.h"
#include "status-led.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(status_led, CONFIG_STATUS_LED_LOG_LEVEL);

// TODO [ ] Determine whether the thread which calls this module may exit,
//          and leave the kernel timer still running.  (This would be ok in
//          most cases, but would represent a resource which the app could no
//          longer turn off or stop using completely.)

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

static struct k_mutex status_led_mtx;

// Forward declarations . . .
static void stop_status_led_timer(void);

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
	int32_t rc = 0;

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

static void stop_status_led_timer(void)
{
	k_timer_stop(&status_led_timer);
}

static void start_status_led_timer(void)
{
	k_timer_start(&status_led_timer, K_MSEC(LED_START_DURATION_MS), K_MSEC(LED_PERIOD_MS));
}

//----------------------------------------------------------------------
// - SECTION - public API
//----------------------------------------------------------------------

int32_t status_led_set_pattern(enum ers_status_led_pattern pattern)
{
	int32_t rc = 0;
	ERS_MUTEX_LOCK(status_led_mtx, CONFIG_STATUS_LED_MUTEX_TIMEOUT_MS, status_led);

	if (!flag_led_initialized) {
		rc = -EFAULT;
		goto unlock;
	}

	led_pattern_fs = pattern;
	start_status_led_timer();

unlock:
	ERS_MUTEX_UNLOCK(status_led_mtx, status_led);
done:
	return 0;
}

int32_t status_led_on(void)
{
	int32_t rc = 0;
	ERS_MUTEX_LOCK(status_led_mtx, CONFIG_STATUS_LED_MUTEX_TIMEOUT_MS, status_led);

	if (!flag_led_initialized) {
		rc = -EFAULT;
		goto unlock;
	}

	stop_status_led_timer();

	rc = gpio_pin_set_dt(&led, 0);
	if (rc < 0) {
		LOG_ERR("Failed to turn on status LED, err %d", rc);
	}

unlock:
	ERS_MUTEX_UNLOCK(status_led_mtx, status_led);
done:
	return rc;
}

int32_t status_led_off(void)
{
	int32_t rc = 0;
	ERS_MUTEX_LOCK(status_led_mtx, CONFIG_STATUS_LED_MUTEX_TIMEOUT_MS, status_led);

	if (!flag_led_initialized) {
		rc = -EFAULT;
		goto unlock;
	}

	stop_status_led_timer();

	rc = gpio_pin_set_dt(&led, 1);
	if (rc < 0) {
		LOG_ERR("Failed to turn on status LED, err %d", rc);
	}

unlock:
	ERS_MUTEX_UNLOCK(status_led_mtx, status_led);
done:
	return rc;
}

int32_t status_led_init(void)
{
	int32_t rc = 0;

	if (flag_led_initialized == true) {
		LOG_WRN("Status LED module already initialized");
		rc = -EFAULT;
		goto done;
	}

	k_mutex_init(&status_led_mtx);

	rc = configure_led();
	if (rc != 0) {
		LOG_ERR("Failed to init GPIO for status LED, err %d", rc);
		goto done;
	}

	k_timer_start(&status_led_timer, K_MSEC(LED_START_DURATION_MS), K_MSEC(LED_PERIOD_MS));
	flag_led_initialized = true;
done:
	return rc;
}
