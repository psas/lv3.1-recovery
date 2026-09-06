#ifndef ERS_ARBITER_H
#define ERS_ARBITER_H

#include "hall-and-ring.h"

#include <zephyr/shell/shell.h>

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

int32_t arbiter_init(void);

/**
 * @brief Set some . . .
 */

void arbiter_cmd_set_default_limits(const struct shell *shell, size_t argc, char **argv);

/**
 * @brief Show present Hall sensor cut-off values, measured empirically and
 *  used to sense lock ring position.
 */

void arbiter_show_hall_state_limits(const struct shell *shell);

/**
 * @brief
 */

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position);

/**
 * @brief Set the interval in milliseconds at which ERS firmware takes Hall
 *  sensor readings and determiines the lock ring position.
 */

int32_t arbiter_set_ring_pos_detection_interval(const uint32_t timeout_ms);

/**
 * @brief Convert an enum value for lock ring position to a string.
 */

char *arbiter_ring_pos_to_str(const enum lock_ring_position pos);

#endif // ERS_ARBITER_H
