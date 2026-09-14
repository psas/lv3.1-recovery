#ifndef ERS_KEEPER_H
#define ERS_KEEPER_H

#include "arbiter.h"
#include "hall-and-ring.h"

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
 * @brief Routine to return a Hall sensor limit for given sensor.
 *
 * @return 0 status code and limit value when sensor id, limit id in bounds.
 * @return -EINVAL otherwise.
 */

int32_t keeper_get_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
				const enum hall_sensor_named_limits limit_idx,
				uint32_t *value);

/**
 * @brief Routine to restore hall sensor limits to default values.
 *
 * @retval 0 on success.
 * @retval -errno as returned by Zephyr settings module, when there are flash
 *  memory access errors.
 */

int32_t keeper_restore_hall_sensor_default_limits(void);

/**
 * @brief Zephyr shell routines to update Hall sensor limit values.
 *
 * @note Parameters are Zephyr shell's standard three parameters, latter two
 *  akin to the historic, conventional parameters of the C "hello world"
 *  program.
 *
 * @retval 0 on success.
 * @retval -EINVAL when:
 *   - sensor id not for Hall sensor 1 or 2
 *   - when sensor named limit not defined
 *   - when limit value outside of ADC reading range
 */

int32_t keeper_cmd_set_limit_v_under(const struct shell *shell, size_t argc, char **argv);
int32_t keeper_cmd_set_limit_inactive(const struct shell *shell, size_t argc, char **argv);
int32_t keeper_cmd_set_limit_between(const struct shell *shell, size_t argc, char **argv);
int32_t keeper_cmd_set_limit_active(const struct shell *shell, size_t argc, char **argv);

/**
 * @brief Store (read) Hall sensor limit values to flash memory.
 *
 * @note Parameters are Zephyr shell's standard three parameters.
 *
 * @retval 0 on success.
 * @retval -errno as returned by Zephyr settings module, when there are flash
 *  memory access errors.
 */

int32_t keeper_cmd_store_hall_limits(const struct shell *shell, size_t argc, char **argv);

/**
 * @brief Retrieve (read) Hall sensor limit values from flash memory.
 *
 * @note Parameters are Zephyr shell's standard three parameters.
 *
 * @retval 0 on success.
 * @retval -errno as returned by Zephyr settings module, when there are flash
 *  memory access errors.
 */

int32_t keeper_cmd_retrieve_hall_limits(const struct shell *shell, size_t argc, char **argv);

// Helper functions to allow for calling Hall limit value retrieval, from the
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

void keeper_set_iso_drogue(const uint32_t value);      // TODO [ ] Check whether needed
void keeper_get_iso_drogue(uint32_t* value);           // TODO [ ] Check whether needed

void keeper_set_iso_main(const uint32_t value);        // TODO [ ] Check whether needed
void keeper_get_iso_main(uint32_t* value);             // TODO [ ] Check whether needed

// Note, this signal represents shore power:
void keeper_set_not_umb_on(const uint32_t value);
void keeper_get_not_umb_on(uint32_t* value);

void keeper_set_not_motor_faila(const uint32_t value); // TODO [ ] Check whether needed
void keeper_get_not_motor_faila(uint32_t* value);      // TODO [ ] Check whether needed

// Analog inputs

void keeper_set_batt_read(const uint32_t value);
void keeper_get_batt_read(uint32_t* value);

// Note, battery voltage reading in millivolts used only for bench top
// reporting.
void keeper_set_batt_millivolts(const uint32_t value);
void keeper_get_batt_millivolts(uint32_t* value);

void keeper_set_motor_isense(const uint32_t value);
void keeper_get_motor_isense(uint32_t* value);

void keeper_set_motor_isense_ma(const uint32_t value);
void keeper_get_motor_isense_ma(uint32_t* value);      // TODO [ ] Check whether needed

// Note, Hall sensor readings in units of millivolts are not needed by ERS
// rocket recovery logic;  they are present for bench top reporting.
void keeper_set_hall_1(const uint32_t value);
void keeper_get_hall_1(uint32_t* value);

void keeper_set_hall_1_mv(const uint32_t value);
void keeper_get_hall_1_mv(uint32_t* value);

void keeper_set_hall_2(const uint32_t value);
void keeper_get_hall_2(uint32_t* value);

void keeper_set_hall_2_mv(const uint32_t value);
void keeper_get_hall_2_mv(uint32_t* value);

/**
 * @brief Write Hall sensor readings, in ADC counts, to keeper data store.
 * @return 0 on success to obtain mutex and to set sensor values
 * @return -FAULT when module not initialized
 */

int32_t keeper_set_both_hall_readings(const uint32_t value_1, const uint32_t value_2); // TODO [ ] Check whether needed

/**
 * @brief Return latest Hall sensor readings, in ADC counts.
 * @return 0 on success to obtain mutex and to get sensor values
 * @return -FAULT when module not initialized
 */

int32_t keeper_get_both_hall_readings(uint32_t *value_1, uint32_t *value_2);

/**
 * @brief Return latest Hall sensor readings, in millivolts.
 * @return 0 on success.
 * @return -FAULT when module not initialized.
 */

int32_t keeper_get_both_hall_readings_in_mv(uint32_t *value_1, uint32_t *value_2);

/**
 * @brief API to set any one of ADC channel readings
 */

int32_t keeper_set_adc_value(const enum ers_adc_values idx, const uint32_t value);

// - DATA GROUP - lock ring

void keeper_set_ring_pos_detection_interval(const uint32_t timeout_ms);
void keeper_get_ring_pos_detection_interval(uint32_t *timeout_ms);

void keeper_set_ring_position(const enum lock_ring_position ring_pos);
void keeper_get_ring_position(enum lock_ring_position *ring_pos);

// Parachute section ring lock and unlock events
void keeper_set_lock_event_count(const uint32_t count);   // <- TODO [ ] check whether used.
void keeper_get_lock_event_count(uint32_t *count);        // <- TODO [ ] check whether used.

void keeper_set_unlock_event_count(const uint32_t count);
void keeper_get_unlock_event_count(uint32_t *count);      // <- TODO [ ] check whether used.

// - DATA GROUP - motor

void keeper_set_DAC_val_for_ring_motor(const uint32_t value);
void keeper_get_DAC_val_for_ring_motor(uint32_t *value);

// - DATA GROUP - ERS summary state data

// (1)
void keeper_set_ring_state(const enum lock_ring_state value);
void keeper_get_ring_state(enum lock_ring_state *value);

// (2)
void keeper_set_batt_decivolts(const uint32_t value);
void keeper_get_batt_decivolts(uint32_t* value);

// (3)
void keeper_set_batt_ok(const uint32_t value);
void keeper_get_batt_ok(uint32_t* value);

// (4) TODO [ ] Replace these two API bodies with those of keeper_get|set_not_umb_on():
void keeper_set_shore_power(const uint32_t value);        // <- TODO [ ] check whether used.
void keeper_get_shore_power(uint32_t* value);             // <- TODO [ ] check whether used.

// (5)
void keeper_set_can_bus_ok(const uint32_t value);
void keeper_get_can_bus_ok(uint32_t* value);

// (6)
void keeper_set_ready_state(const uint32_t value);
void keeper_get_ready_state(uint32_t* value);

// ERS diagnostics

void keeper_set_diag_periodic(void);
void keeper_clear_diag_periodic(void);
void keeper_get_diag_mode(uint32_t* value);

#endif // ERS_KEEPER_H
