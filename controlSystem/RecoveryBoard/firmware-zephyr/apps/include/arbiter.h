#ifndef ERS_ARBITER_H
#define ERS_ARBITER_H

#include "hall-and-ring.h"

#include <zephyr/shell/shell.h>

/**
 * @brief ERS arbiter initialization routine.
 *
 * @warning This routine must be called to make arbiter module ready for use.
 */

int32_t arbiter_init(void);

/**
 * @brief Parse string-wise Hall sensor abbreviation and return a numeric
 *  reference to it, based on an enum of Hall sensors in the ERS system.
 *
 * @param sensor_name is the string-wise abbreviation "s1" or "s2", refers to
 *   one of Hall sensor 1 and Hall sensor 2.
 * @param sensor_idx is a pointer to a caller variable, meant to hold an
 *   effective index to the Hall sensor on which to act.
 *
 * @retval 0 on success
 * @retval -EINVAL if sensor abbreviation not one of "s1" and "s2".
 */

int32_t determine_which_sensor(const char *sensor_name, enum hall_sensor_instances *sensor_idx);

/**
 * @brief Calculate maximum motor current, based on DAC output setting value.
 *
 * @retval 0 on success.
 * @retval -EFAULT on API mutex timeout.
 */

int32_t arbiter_calc_max_motor_drive_current(void);

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
 * @brief Assign Hall sensor default cut-off values to app run-time variables.
 * @note These values may likely become inaccurate when the physical, ERS lock
 *  ring assembly is handled.  Small changes in the position of the Hall
 *  sensors and an associated permanent magnet with each sensor lead to
 *  notable changes in Hall readings, which correspond to ring states.
 *
 * @note The three parameters of this API are standard to Zephyr shell
 *  commands.
 *
 * @param shell Pointer to Zephyr shell construct instance in the app.
 * @param argc Count of input tokens, separated by white space.
 * @param argv array of pointers to input tokens.
 */

void arbiter_cmd_set_default_limits(const struct shell *shell, size_t argc, char **argv);

/**
 * @brief Show present Hall sensor cut-off values, measured empirically and
 *  used to sense lock ring position.
 *
 * @param shell Pointer to Zephyr shell construct instance in the app.
 */

void arbiter_show_hall_state_limits(const struct shell *shell);

/**
 * @brief Calculate system battery voltage from latest ADC reading.
 *
 * @note Calcuates battery voltage in both millivolts and tenths of a volt.
 *
 * @retval 0 on success.
 * TODO [ ] Add mutex to arbiter and return -EAGAIN
 */

int32_t calc_battery_voltage(void);

/**
 * @brief Determine lock ring postion, and state.  Return ring position.
 *
 * @param pointer to caller variable to hold lock ring position.
 *
 * @retval 0 on success, ring position in variable pointed to by ring_position.
 * TODO [ ] @retval -EAGAIN when mutex lock request times out.
 * @return . . .
 */

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position);

/**
 * @brief Convert an enum value for lock ring position to a string.
 *
 * @param pos An integer value representing lock ring physical position.
 *
 * @return A string-wise name for the physical ring position.  Possible strings
 *  include a string to indicate when position is unknown.
 */

char *arbiter_ring_pos_to_str(const enum lock_ring_position pos);

#endif // ERS_ARBITER_H
