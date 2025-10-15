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

int32_t ers_init_arbiter(void);

void arbiter_set_v_under_cutoff(const uint32_t value);
void arbiter_set_inactive_cutoff(const uint32_t value);
void arbiter_set_between_cutoff(const uint32_t value);
void arbiter_set_active_cutoff(const uint32_t value);

void arbiter_get_v_under_cutoff(uint32_t *value);
void arbiter_get_inactive_cutoff(uint32_t *value);
void arbiter_get_between_cutoff(uint32_t *value);
void arbiter_get_active_cutoff(uint32_t *value);

void shell_wrapper_set_v_under_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_inactive_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_between_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_active_cutoff(const struct shell *shell, size_t argc, char **argv);

void arbiter_show_hall_state_cutoffs(const struct shell *shell);

// TODO [ ] add API to restore default Hall state cutoff values

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position);

char *ring_pos_to_str(enum lock_ring_position pos);

#endif // ERS_ARBITER_H
