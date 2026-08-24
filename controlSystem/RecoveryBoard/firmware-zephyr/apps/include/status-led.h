#ifndef ERS_STATUS_LED
#define ERS_STATUS_LED

#include <stdint.h>

enum ers_status_led_pattern {
	STATUS_LED_OFF,
	STATUS_LED_HEARTBEAT,
	STATUS_LED_RING_STATE_UNKNOWN,
};

int32_t status_led_init(void);

int32_t status_led_set_pattern(enum ers_status_led_pattern pattern);

#endif // ERS_STATUS_LED
