#ifndef ERS_KEEPER_H
#define ERS_KEEPER_H

#include <arbiter.h>
#include "ers-config.h"

/**
 * @note ERS "Keeper" module, to hold and share most ERS board run time data.
 *
 * @note This module categorizes ERS data into three general kinds:
 *
 *   (1) configuration data
 *
 *   (2) readings (direct and derived)
 *
 *   (3) state data
 *
 *   Some examples of ERS config data include Hall sensor limits, diagnostic
 *   development message settings, and sensor reading intervals.  Some
 *   example readings include digital GPIO pins and readings of on-chip ADC
 *   channels.
 *
 *   Some examples of ERS board state include CAN bus run time health,
 *   identified by the presence of certain message packets from the larger
 *   rocket avionics system.
 */

// Module init API

/**
 * @warning Keeper module initialization routine must be called before use
 *   of this module.
 *
 * @return 0 without condition.
 */

int32_t keeper_init(void);

//----------------------------------------------------------------------
// - SECTION - ERS configuration
//----------------------------------------------------------------------

/**
 * @brief Routine to set a given Hall sensor limit, a cutoff value
 *   measured in ADC counts, for each hall sensor in an ERS board.
 *
 * @return 0 status code when sensor id, limit id in bounds.
 * @return -EINVAL otherwise.
 */

int32_t set_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
                                const enum hall_sensor_named_limits limit_idx,
                                const uint32_t val);

/**
 * @brief Routine to return a Hall sensor limit for given sensor.
 *
 * @return 0 status code and limit value when sensor id, limit id in bounds.
 * @return -EINVAL otherwise.
 */

int32_t get_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
                                const enum hall_sensor_named_limits limit_idx,
                                uint32_t *value);

/**
 * @brief Routine to restore hall sensor limits to default values.
 */

int32_t keeper_set_hall_sensor_default_limits(void);

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

int32_t cmd_save_hall_limits_to_flash(const struct shell *shell, size_t argc, char **argv);
int32_t cmd_retrieve_hall_limits_from_flash(const struct shell *shell, size_t argc, char **argv);

// Helper functions to allow for calling Hall cut-off value retrieval from the
// ERS app as well as from the CLI of the app:
int32_t keeper_retrieve_hall_1_limits(void);
int32_t keeper_retrieve_hall_2_limits(void);

//----------------------------------------------------------------------
// - SECTION - ERS readings
//----------------------------------------------------------------------

// Note, next two enumeration names refer to ADC values and not channels, as
// two values per active channel are stored:  (1) ADC counts and (2) reading
// converted to millivolts.

// Note, these enum values are used in a switch construct, so that they serve as
// something like an index.

enum ers_adc_values {
        ADC_READING_HALL_1,        // ADC_IN0
        ADC_READING_HALL_2,        // ADC_IN1
        ADC_READING_BATT_READ,     // ADC_IN8
        ADC_READING_MOTOR_ISENSE,  // ADC_IN9
        ADC_CHANNEL_COUNT
};

enum ers_adc_values_in_mv {
        ADC_READING_HALL_1_MV,
        ADC_READING_HALL_2_MV,
        ADC_READING_BATT_READ_MV,
        ADC_READING_MOTOR_ISENSE_MV
};

#define IDX_START_MV_READINGS ADC_CHANNEL_COUNT

// Digital inputs

void ekset_iso_drogue(const uint32_t value);
void ekset_iso_main(const uint32_t value);
void ekset_not_umb_on(const uint32_t value);
void ekset_not_motor_faila(const uint32_t value);

void ekget_iso_drogue(uint32_t* value);
void ekget_iso_main(uint32_t* value);
void ekget_not_umb_on(uint32_t* value);
void ekget_not_motor_faila(uint32_t* value);

// Analog inputs

void ekset_batt_read(const uint32_t value);
void ekset_motor_isense(const uint32_t value);
void ekset_hall_1(const uint32_t value);
void ekset_hall_2(const uint32_t value);

void ekset_batt_read_mv(const uint32_t value);
void ekset_motor_isense_ma(const uint32_t value);
void ekset_hall_1_mv(const uint32_t value);
void ekset_hall_2_mv(const uint32_t value);

void ekset_batt_read_dv(const uint32_t value);

/**
 * @return 0 on success to obtain mutex and to set sensor values
 * @return -ESRCH when module not initialized
 */

int32_t ekset_both_hall_sensors(const uint32_t value_1, const uint32_t value_2);

/**
 * @brief API to set any one of ADC channel readings
 */

int32_t ekset_adc_value(const enum ers_adc_values idx, const uint32_t value);

/**
 * @brief API to set any one of ADC channel readings converted to millivolts
 */

int32_t ekset_adc_value_in_mv(const enum ers_adc_values_in_mv idx, const uint32_t val);

void ekget_batt_read(uint32_t* value);
void ekget_motor_isense(uint32_t* value);
void ekget_hall_1(uint32_t* value);
void ekget_hall_2(uint32_t* value);

void ekget_batt_read_mv(uint32_t* value);
void ekget_motor_isense_mv(uint32_t* value);
void ekget_hall_1_mv(uint32_t* value);
void ekget_hall_2_mv(uint32_t* value);

void ekget_batt_read_dv(uint32_t* value);

// - DATA GROUP - (3) locking ring

/**
 * @return 0 on success to obtain mutex and to get sensor values
 * @return -ESRCH when module not initialized
 */
int32_t ekget_both_hall_sensors(uint32_t *value_1, uint32_t *value_2);

void set_ring_pos_detection_interval(const uint32_t timeout_ms);

void get_ring_pos_detection_interval(uint32_t *timeout_ms);

void set_detected_ring_position(const enum lock_ring_position ring_pos);

void get_detected_ring_position(enum lock_ring_position *ring_pos);

// Parachute section ring lock and unlock events
void set_ring_lock_event_count(const uint32_t count);
void set_ring_unlock_event_count(const uint32_t count);
void get_ring_lock_event_count(uint32_t *count);
void get_ring_unlock_event_count(uint32_t *count);

// - DATA GROUP - (4) motor

void ekset_DAC_setting_ring_lock(const uint32_t value);
void ekget_DAC_setting_ring_lock(uint32_t *value);

// - DATA GROUP - (6) ERS summary state data

void ekset_ring_status(const enum lock_ring_state value);
void ekset_batt_ok(const uint32_t value);
void ekset_shore_power_ok(const uint32_t value);
void ekset_can_bus_ok(const uint32_t value);
void ekset_ready_state(const uint32_t value);

void ekget_ring_status(enum lock_ring_state *value);
void ekget_batt_ok(uint32_t* value);
void ekget_shore_power_ok(uint32_t* value);
void ekget_can_bus_ok(uint32_t* value);
void ekget_ready_state(uint32_t* value);

// ERS diagnostics

void keeper_set_diag_periodic(void);
void keeper_clear_diag_periodic(void);
void keeper_get_diag_mode(uint32_t* value);

#endif // ERS_KEEPER_H
