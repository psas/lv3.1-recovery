#ifndef ERS_STATUS_LED
#define ERS_STATUS_LED

#include <stdint.h>

enum ers_status_led_pattern {
	STATUS_LED_OFF,
	STATUS_LED_HEARTBEAT,
	STATUS_LED_SOS,
	STATUS_LED_RING_STATE_UNKNOWN,
	STATUS_LED_PATTERN_UNDEFINED,
};

int32_t status_led_init(void);

int32_t status_led_set_pattern(enum ers_status_led_pattern pattern);

/**
 * @brief Turn on status LED.
 *
 * @note Routine intended for developent time work.  Works best when LED
 *  patterns are effectively disabled.  There is a "no blinking" pattern
 *  in place for this.
 *
 * @retval 0 on success.
 * @retval -EFAULT on module not initialized.
 * TODO [ ] Add:  @retval -EAGAIN on API mutex timeout.
 */

int32_t status_led_on(void);

/**
 * @brief Turn off status LED.
 *
 * @note Routine intended for developent time work.  Works best when LED
 *  patterns are effectively disabled.  There is a "no blinking" pattern
 *  in place for this.
 *
 * @retval 0 on success.
 * @retval -EFAULT on module not initialized.
 * TODO [ ] Add:  @retval -EAGAIN on API mutex timeout.
 */

int32_t status_led_off(void);

#endif // ERS_STATUS_LED
