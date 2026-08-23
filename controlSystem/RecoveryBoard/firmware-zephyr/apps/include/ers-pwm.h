#ifndef ERS_PWM_H
#define ERS_PWM_H

#include <stdint.h>

enum pwm_audio_pattern {
	PWM_APP_SILENT,
	PWM_APP_BANNER_PATTERN,
	PWM_APP_HEARTBEAT,
	PWM_ROCKET_READY,
};

/**
 * @brief Function to take a caller's requested pattern and start that playing.
 */

int32_t pwm_play_pattern(enum pwm_audio_pattern);

int32_t pwm_init(void);

#endif // ERS_PWM_H
