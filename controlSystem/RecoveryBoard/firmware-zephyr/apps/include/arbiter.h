#ifndef ERS_ARBITER_H
#define ERS_ARBITER_H

#include <zephyr/shell/shell.h>

enum hall_sensor_state {
	HALL_OUTPUT_UNDER_VOLTAGE,
	HALL_OUTPUT_INACTIVE,
	HALL_OUTPUT_BETWEEN,
	HALL_OUTPUT_ACTIVE,
	HALL_OUTPUT_OVER_VOLTAGE,
	HALL_OUTPUT_UNKNOWN
};

enum lock_ring_position {
	RING_LOCKED,
	RING_BETWEEN_L_AND_U,
	RING_UNLOCKED,
	RING_LOCKED_FULLY_QUALIFIED,
	RING_BETWEEN_FULLY_QUALIFIED,
	RING_UNLOCKED_FULLY_QUALIFIED,
	RING_POSITION_UNKNOWN
};

/**
 * @brief ERS arbiter initialization routine.
 *
 * @warning This routine must be called to make arbiter module ready for use.
 */

int32_t ers_init_arbiter(void);

#if 0
void arbiter_set_v_under_cutoff(const uint32_t value);
void arbiter_set_inactive_cutoff(const uint32_t value);
void arbiter_set_between_cutoff(const uint32_t value);
void arbiter_set_active_cutoff(const uint32_t value);

void arbiter_get_v_under_cutoff(uint32_t *value);
void arbiter_get_inactive_cutoff(uint32_t *value);
void arbiter_get_between_cutoff(uint32_t *value);
void arbiter_get_active_cutoff(uint32_t *value);
#endif // 0

int32_t sw_set_limit_v_under(const struct shell *shell, size_t argc, char **argv);
int32_t sw_set_limit_inactive(const struct shell *shell, size_t argc, char **argv);
int32_t sw_set_limit_between(const struct shell *shell, size_t argc, char **argv);
int32_t sw_set_limit_active(const struct shell *shell, size_t argc, char **argv);

void sw_set_default_limits(const struct shell *shell, size_t argc, char **argv);

void arbiter_show_hall_state_limits(const struct shell *shell);

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position);

char *ring_pos_to_str(enum lock_ring_position pos);

#endif // ERS_ARBITER_H
