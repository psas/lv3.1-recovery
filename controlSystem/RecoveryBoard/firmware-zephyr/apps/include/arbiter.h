#ifndef ERS_ARBITER_H
#define ERS_ARBITER_H

#include <zephyr/shell/shell.h>

// To provide `enum hall_sensor_instances`:
#include "ers-config.h"

/**
 * @brief TODO [ ] complete this stub comment block.
 */

int32_t determine_which_sensor(const char *sensor_name, enum hall_sensor_instances *sensor_idx);

enum lock_ring_position {
	RING_POS_LOCKED,
	RING_POS_BETWEEN_L_AND_U,
	RING_POS_UNLOCKED,
	RING_POS_LOCKED_FULLY_QUALIFIED,
	RING_POS_BETWEEN_FULLY_QUALIFIED,
	RING_POS_UNLOCKED_FULLY_QUALIFIED,
	RING_POS_UNKNOWN
};

/**
 * @note Ring state is a simplified version of lock_ring_position, and is
 *   sent in the given ERS board's CAN heartbeat / status message.
 */

// TODO [ ] Find out from team which state to apply when ring position is not
//           known.

enum lock_ring_state {
	RING_STATE_UNKNOWN,
	RING_STATE_UNLOCKED,
	RING_STATE_BETWEEN,
	RING_STATE_LOCKED,
	RING_STATE_ERROR
};

/**
 * @brief ERS arbiter initialization routine.
 *
 * @warning This routine must be called to make arbiter module ready for use.
 */

int32_t ers_init_arbiter(void);

#if 0
/**
 * @brief Following four routines implement custom Zephyr shell commands.  As
 *   arguments these routines expect:
 *
 * @param ["s1"|"s2"] to indicate which of two Hall sensors the limit applies.
 * @param [0..4095] an ADC count within the 12-bit ADC range of possible values.
 */

int32_t cmd_set_limit_v_under(const struct shell *shell, size_t argc, char **argv);
int32_t cmd_set_limit_inactive(const struct shell *shell, size_t argc, char **argv);
int32_t cmd_set_limit_between(const struct shell *shell, size_t argc, char **argv);
int32_t cmd_set_limit_active(const struct shell *shell, size_t argc, char **argv);

#endif // 0 . . . moving APIs to keeper module

void cmd_set_default_limits(const struct shell *shell, size_t argc, char **argv);

void arbiter_show_hall_state_limits(const struct shell *shell);

// Ring position related
int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position);

int32_t update_ring_position_detection_timer(const uint32_t timeout_ms);

char *ring_pos_to_str(const enum lock_ring_position pos);

#endif // ERS_ARBITER_H
