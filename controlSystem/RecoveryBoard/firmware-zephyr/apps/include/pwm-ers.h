#ifndef ERS_PWM_H
#define ERS_PWM_H

#include <stdint.h>

enum pwm_audio_pattern {
	PWM_APP_SILENT,
	PWM_APP_BANNER_PATTERN,
	PWM_APP_15_NOTES,
	PWM_APP_HEARTBEAT,
	PWM_ROCKET_READY,
};

/**
 * @brief Play a defined sequence (pattern) of audio tones.
 *
 * retval 0 on success.
 * retval -EINVAL when sequence not defined.
 */

int32_t pwm_play_pattern(enum pwm_audio_pattern);

/**
 * @brief Initialize ERS pulse width modulation module, for audio output
 *  support.
 *
 * retval 0 on success.
 * retval -EFAULT when the PWM thread fails to start.
 * retval -ENODEV when PWM device reported as not ready.
 * retval -ENOTSUP when a PWM channel configuration fails.
 */

int32_t pwm_init(void);

#endif // ERS_PWM_H
