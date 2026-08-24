/**
 * @file
 * @brief ERS keeper module, to hold and to share run time state.
 */

// TODO [ ] Standardize public API names to begin with 'keeper_'.
// TODO [x] Double quote local header filenames here and in all ERS sources.
#include "arbiter.h"
#include "ers-config.h"
#include "keeper.h"
#include "status-led.h"
#include "settings-ers.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(keeper, LOG_LEVEL_INF);

/**
 * @defgroup digital_inputs
 */

static atomic_t iso_drogue = ATOMIC_INIT(0);
static atomic_t iso_main = ATOMIC_INIT(0);
static atomic_t not_umb_on = ATOMIC_INIT(0);

/**
 * @defgroup battery
 */

static atomic_t batt_read = ATOMIC_INIT(0);
static atomic_t batt_read_mv = ATOMIC_INIT(0);
// TODO [ ] Refactor battery millivolt to decivolt conversion to occur
//   after calls to get battery voltage:
static atomic_t batt_read_dv = ATOMIC_INIT(0);

/**
 * @defgroup motor_related
 */

static atomic_t not_motor_faila = ATOMIC_INIT(0);

static atomic_t motor_isense = ATOMIC_INIT(0);

static atomic_t motor_isense_mv = ATOMIC_INIT(0);

static atomic_t dac_setting_ring_lock = ATOMIC_INIT(0);

/**
 * @defgroup sensors
 *
 * @note Hall sensor limits are empirically determined readings, in this case
 *   ADC counts, above which or below which the firmware is written to
 *   treat such crossing as a physical state change in the lock ring.
 *   Further there are two limits which we / firmware treat as sensor error
 *   conditions.  Those values we should never see from an intact, working
 *   sensor.
 */

static atomic_t hall_1 = ATOMIC_INIT(0);
static atomic_t hall_2 = ATOMIC_INIT(0);
static atomic_t hall_1_mv = ATOMIC_INIT(0);
static atomic_t hall_2_mv = ATOMIC_INIT(0);

struct hall_sensor_limits {
	atomic_t v_under;
	atomic_t inactive;
	atomic_t between;
	atomic_t active;
};

static struct hall_sensor_limits hall_sensor_fs[HALL_SENSOR_COUNT];

// TODO [ ] Determine whether var 'ring_pos_interval' actually used, only seems to be referenced
//          in this file:
// App determines lock ring position at this interval of time:
static atomic_t ring_pos_interval = ATOMIC_INIT(0);

// Counts of times ring locked and unlocked:
static atomic_t ring_lock_events = ATOMIC_INIT(0);
static atomic_t ring_unlock_events = ATOMIC_INIT(0);

// Summary state variables (values usually determined by tests of simpler data):

// TODO [ ] Consider factoring summary state variables into a structure,
//  this may improve code readability and mainenance:

static atomic_t ring_status = ATOMIC_INIT(0);
// QUESTION - put battery voltage in struct of ERS states?
static atomic_t batt_ok = ATOMIC_INIT(0);
static atomic_t shore_power_ok = ATOMIC_INIT(0);
static atomic_t can_bus_ok = ATOMIC_INIT(0);
static atomic_t ready_state = ATOMIC_INIT(0);

// TODO [ ] create public API getter for CAN module to access ERS summary state.

struct ers_summary_state {
	atomic_t ring_position;
	atomic_t battery_voltage;
	atomic_t battery_ok;
	atomic_t shore_power_ok;
	atomic_t can_bus_ok;
	atomic_t ready_flag;
};

static struct ers_summary_state summary_state;

/**
 * @brief Struct of structs, gathers most ERS board state, configuration, 
 *   and sensor readings in one data structure.
 */
 
struct ers_config_and_state {
	struct hall_sensor_limits *hall_1_limit;
	struct hall_sensor_limits *hall_2_limit;
	struct ers_summary_state *summary_state;
};

// Support run time toggling of diagnostics which share UART with Zephyr shell:
static atomic_t ers_diag_flag_fs = ATOMIC_INIT(0);

//----------------------------------------------------------------------
// - SECTION - module concurrency and state
//----------------------------------------------------------------------

// Provide a mutex to assure that both Hall sensors are updated without anyone
// reading their latest values in the middle of this pair of updates:
struct k_mutex hall_sensors_mtx;

// Flag to indiciate that this module is initialized:
static bool keeper_initialized_fs = false;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (1) battery
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Battery reading in ADC counts
void ekset_batt_read(const uint32_t value)
{
	atomic_set(&batt_read, (atomic_val_t)value);
}

void ekget_batt_read(uint32_t* value)
{
	*value = atomic_get(&batt_read);
}

// Battery reading in millivolts
void ekset_batt_read_mv(const uint32_t value)
{
	atomic_set(&batt_read_mv, (atomic_val_t)value);
}

void ekget_batt_read_mv(uint32_t* value)
{
	*value = atomic_get(&batt_read_mv);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (2) digital inputs
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ekset_iso_drogue(const uint32_t value)
{
	atomic_set(&iso_drogue, (atomic_val_t)value);
}

void ekset_iso_main(const uint32_t value)
{
	atomic_set(&iso_main, (atomic_val_t)value);
}

void ekset_not_umb_on(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

void ekget_iso_drogue(uint32_t* value)
{
	*value = atomic_get(&iso_drogue);
}

void ekget_iso_main(uint32_t* value)
{
	*value = atomic_get(&iso_main);
}

void ekget_not_umb_on(uint32_t* value)
{
	*value = atomic_get(&not_umb_on);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (2 1/2) analog inputs not categorized
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int32_t ekset_adc_value(const enum ers_adc_values idx, const uint32_t val)
{
	switch (idx)
	{
        case ADC_READING_BATT_READ:
		ekset_batt_read(val);
		break;
        case ADC_READING_MOTOR_ISENSE:
		ekset_motor_isense(val);
		break;
	case ADC_READING_HALL_1:
		ekset_hall_1(val);
		break;
        case ADC_READING_HALL_2:
		ekset_hall_2(val);
		break;
	default:
		LOG_ERR("Asked to store value for undefined ADC channel %d", idx);
		return -EINVAL;
	}

	return 0;
}

int32_t ekset_adc_value_in_mv(const enum ers_adc_values_in_mv idx, const uint32_t val)
{
	switch (idx)
	{
        case ADC_READING_BATT_READ_MV:
		ekset_batt_read_mv(val);
		break;
        case ADC_READING_MOTOR_ISENSE_MV:
		ekset_motor_isense_ma(val);
		break;
        case ADC_READING_HALL_1_MV:
		ekset_hall_1_mv(val);
		break;
        case ADC_READING_HALL_2_MV:
		ekset_hall_2_mv(val);
		break;
	default:
		LOG_ERR("Asked to store value for undefined ADC channel %d", idx);
		return -EINVAL;
	}

	return 0;
}

//----------------------------------------------------------------------
// - SECTION - Hall sensor limits and states
//----------------------------------------------------------------------

// Routines set, get, store and retrieve Hall sensor limits

// TODO [ ] Add check of 'endptr' to determine whether we got valid numeric input,
//  in all routines which call strtol():

int32_t cmd_set_limit_v_under(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_instances sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'v_under' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_UNDER, value);

	return 0;
}

int32_t cmd_set_limit_inactive(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_instances sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'inactive' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_INACTIVE, value);

	return 0;
}

int32_t cmd_set_limit_between(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_instances sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'between' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_BETWEEN, value);

	return 0;
}

int32_t cmd_set_limit_active(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_instances sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'active' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_ACTIVE, value);
	return 0;
}

int32_t cmd_save_hall_limits_to_flash(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;
	int32_t rc = 0;

	// Call keeper to obtain hall limits:
	get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_UNDER, &v_under_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_INACTIVE, &inactive_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_BETWEEN, &between_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_ACTIVE, &active_limit);

	// TODO [ ] Develop a readable way to capture follolwing return values in a bitwise
	//  fashion, for aggregate check of success or failure of all flash store
	//  operations:
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_1), (const void *)v_under_limit,
				sizeof(v_under_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_2), (const void *)inactive_limit,
				sizeof(inactive_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_3), (const void *)between_limit,
				sizeof(between_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_4), (const void *)active_limit,
				sizeof(active_limit));

	get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_UNDER, &v_under_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_INACTIVE, &inactive_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_BETWEEN, &between_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_ACTIVE, &active_limit);

	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_1), (const void *)v_under_limit,
				sizeof(v_under_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_2), (const void *)inactive_limit,
				sizeof(inactive_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_3), (const void *)between_limit,
				sizeof(between_limit));
	rc = store_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_4), (const void *)active_limit,
				sizeof(active_limit));

	//
	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor limit values store to flash.\n");
	return rc;
}

int32_t keeper_retrieve_hall_1_limits(void)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;
	int32_t rc = 0;

	// Call keeper to obtain hall limits:
	// shell_fprintf(shell, SHELL_NORMAL, "- STUB -\n");
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_1), &v_under_limit,
					sizeof(v_under_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_2), &inactive_limit,
					sizeof(inactive_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_3), &between_limit,
					sizeof(between_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_4), &active_limit,
					sizeof(active_limit));

	// Store retrieved Hall sensor limits to SRAM for run time use:
	set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_UNDER, v_under_limit);
	set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_INACTIVE, inactive_limit);
	set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_BETWEEN, between_limit);
	set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_ACTIVE, active_limit);

	return rc;
}

int32_t keeper_retrieve_hall_2_limits(void)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;
	int32_t rc = 0;

	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_1), &v_under_limit,
					sizeof(v_under_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_2), &inactive_limit,
					sizeof(inactive_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_3), &between_limit,
					sizeof(between_limit));
	rc = retrieve_ers_setting(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_4), &active_limit,
					sizeof(active_limit));

	// Store retrieved Hall sensor limits to SRAM for run time use:
	set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_UNDER, v_under_limit);
	set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_INACTIVE, inactive_limit);
	set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_BETWEEN, between_limit);
	set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_ACTIVE, active_limit);

	return rc;
}

int32_t cmd_retrieve_hall_limits_from_flash(const struct shell *shell, size_t argc, char **argv)
{
	int32_t rc = 0;

	rc = keeper_retrieve_hall_1_limits();

	// TODO [ ] Name each limit in this response part of this command:
//	shell_fprintf(shell, SHELL_NORMAL, "Retrieved Hall sensor 1 limits: %u, %u, %u, %u",
//			v_under_limit, inactive_limit, between_limit, active_limit);

	rc = keeper_retrieve_hall_2_limits();

	// TODO [ ] Name each limit in this response part of this command:
//	shell_fprintf(shell, SHELL_NORMAL, "Retrieved Hall sensor 2 limits: %u, %u, %u, %u",
//			v_under_limit, inactive_limit, between_limit, active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor limits retrieved.  "
			"Enter 'hall show' to see them\n");
	return rc;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (3) locking ring
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Getters

void ekget_hall_1(uint32_t* value)
{
	*value = atomic_get(&hall_1);
}

void ekget_hall_2(uint32_t* value)
{
	*value = atomic_get(&hall_2);
}

void ekget_hall_1_mv(uint32_t* value)
{
	*value = atomic_get(&hall_1_mv);
}

void ekget_hall_2_mv(uint32_t* value)
{
	*value = atomic_get(&hall_2_mv);
}

// Setters

void ekset_hall_1(const uint32_t value)
{
	atomic_set(&hall_1, (atomic_val_t)value);
}

void ekset_hall_2(const uint32_t value)
{
	atomic_set(&hall_2, (atomic_val_t)value);
}

void ekset_hall_1_mv(const uint32_t value)
{
	atomic_set(&hall_1_mv, (atomic_val_t)value);
}

void ekset_hall_2_mv(const uint32_t value)
{
	atomic_set(&hall_2_mv, (atomic_val_t)value);
}

/**
 * @brief Function to store both Hall sensor readings with mutual exclusion
 *   to assure these values are read only when both are up to date.
 */

int32_t ekset_both_hall_sensors(const uint32_t value_1, const uint32_t value_2)
{
	int32_t rc = 0;
	if (!keeper_initialized_fs)
	{
		LOG_ERR("Data keeper module not initialized!");
		return -ESRCH;
	}

	k_mutex_lock(&hall_sensors_mtx, K_FOREVER);
	if (rc != 0)
	{
		LOG_ERR("Failed to lock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	ekset_hall_1(value_1);
	ekset_hall_2(value_2);

	k_mutex_unlock(&hall_sensors_mtx);
	if (rc != 0)
	{
		LOG_ERR("Failed to lock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	return 0;
}

int32_t ekget_both_hall_sensors(uint32_t *value_1, uint32_t *value_2)
{
	int32_t rc = 0;

	if (!keeper_initialized_fs)
	{
		LOG_ERR("Data keeper module not initialized!");
		*value_1 = atomic_get(&hall_1);
		*value_2 = atomic_get(&hall_2);
		return -ESRCH;
	}

// TODO [ ] Watch behavior when issuing lock and unlock ring commands, try to
//  determine why mutex lock and unlock calls were commented out here as of
//  2026-01-04:
	k_mutex_lock(&hall_sensors_mtx, K_FOREVER);
	if (rc != 0)
	{
		LOG_ERR("Failed to lock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	ekget_hall_1_mv(value_1);
	ekget_hall_2_mv(value_2);

	k_mutex_unlock(&hall_sensors_mtx);
	if (rc != 0)
	{
		LOG_ERR("Failed to unlock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	return 0;
}

/**
 * @note Hall sensor limits help us to categorize ADC reading sub-ranges
 *   into physical positions of the lock ring drive gear relative to the
 *   airframe.  Here define setter and getter APIs to support run time
 *   adjustments to these readings sub-range limits.
 */

int32_t set_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
				const enum hall_sensor_named_limits limit_idx,
				const uint32_t value)
{
	if ((sensor_idx < 0) || (sensor_idx >= HALL_SENSOR_COUNT))
	{
		return -EINVAL;
	}

	if ((limit_idx < 0) || (limit_idx >= HALL_SENSOR_LIMIT_COUNT))
	{
		return -EINVAL;
	}

	switch (limit_idx) {
	case HALL_LIMIT_V_UNDER:
		atomic_set(&hall_sensor_fs[sensor_idx].v_under, value);
		break;
        case HALL_LIMIT_V_INACTIVE:
		atomic_set(&hall_sensor_fs[sensor_idx].inactive, value);
		break;
        case HALL_LIMIT_V_BETWEEN:
		atomic_set(&hall_sensor_fs[sensor_idx].between, value);
		break;
        case HALL_LIMIT_V_ACTIVE:
		atomic_set(&hall_sensor_fs[sensor_idx].active, value);
		break;
	default:
	}

	return 0;
}

int32_t get_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
				const enum hall_sensor_named_limits limit_idx,
				uint32_t *value)
{
	if ((sensor_idx < 0) || (sensor_idx >= HALL_SENSOR_COUNT))
	{
		return -EINVAL;
	}

	if ((limit_idx < 0) || (limit_idx >= HALL_SENSOR_LIMIT_COUNT))
	{
		return -EINVAL;
	}

	switch (limit_idx) {
	case HALL_LIMIT_V_UNDER:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].v_under);
		break;
        case HALL_LIMIT_V_INACTIVE:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].inactive);
		break;
        case HALL_LIMIT_V_BETWEEN:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].between);
		break;
        case HALL_LIMIT_V_ACTIVE:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].active);
		break;
	default:
	}

	return 0;
}

void set_ring_pos_detection_interval(const uint32_t timeout_ms)
{
	atomic_set(&ring_pos_interval, (atomic_val_t)timeout_ms);
}

void get_ring_pos_detection_interval(uint32_t *timeout_ms)
{
	*timeout_ms = atomic_get(&ring_pos_interval);
}

void set_detected_ring_position(const enum lock_ring_position ring_pos)
{
	atomic_set(&summary_state.ring_position, (atomic_val_t)ring_pos);
}

void get_detected_ring_position(enum lock_ring_position *ring_pos)
{
	*ring_pos = atomic_get(&summary_state.ring_position);
}

// Parachute section ring lock and unlock events

void set_ring_lock_event_count(const uint32_t count)
{
	atomic_set(&ring_lock_events, (atomic_val_t)count);
}

void set_ring_unlock_event_count(const uint32_t count)
{
	atomic_set(&ring_unlock_events, (atomic_val_t)count);
}

void get_ring_lock_event_count(uint32_t *count)
{
	*count = atomic_get(&ring_lock_events);
}

void get_ring_unlock_event_count(uint32_t *count)
{
	*count = atomic_get(&ring_unlock_events);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (4) motor
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// motor current reading in ADC counts
void ekset_motor_isense(const uint32_t value)
{
	atomic_set(&motor_isense, (atomic_val_t)value);
}

void ekget_motor_isense(uint32_t* value)
{
	*value = atomic_get(&motor_isense);
}

// motor current reading in milliamps
void ekset_motor_isense_ma(const uint32_t value)
{
	atomic_set(&motor_isense_mv, (atomic_val_t)value);
}

void ekget_motor_isense_mv(uint32_t* value)
{
	*value = atomic_get(&motor_isense_mv);
}

// motor digitnal status signal out
void ekset_not_motor_faila(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

void ekget_not_motor_faila(uint32_t* value)
{
	*value = atomic_get(&not_motor_faila);
}

// DAC setting to use for ring lock and ring unlock motor actuations
void ekset_DAC_setting_ring_lock(const uint32_t value)
{
	atomic_set(&dac_setting_ring_lock, (atomic_val_t)value);
}

void ekget_DAC_setting_ring_lock(uint32_t *value)
{
	*value = atomic_get(&dac_setting_ring_lock);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (6) ERS summary state data
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Lock ring status
void ekset_ring_status(const enum lock_ring_state value)
{
	atomic_set(&ring_status, (atomic_val_t)value);
}

void ekget_ring_status(enum lock_ring_state *value)
{
	*value = atomic_get(&ring_status);
}

// Battery reading in decivolts
void ekset_batt_read_dv(const uint32_t value)
{
	atomic_set(&batt_read_dv, (atomic_val_t)value);
}

void ekget_batt_read_dv(uint32_t *value)
{
	*value = atomic_get(&batt_read_dv);
}

// Battery ok flag
void ekset_batt_ok(const uint32_t value)
{
	atomic_set(&batt_ok, (atomic_val_t)value);
}

void ekget_batt_ok(uint32_t* value)
{
	*value = atomic_get(&batt_ok);
}

// Shore power ok flag
void ekset_shore_power_ok(const uint32_t value)
{
	atomic_set(&shore_power_ok, (atomic_val_t)value);
}

void ekget_shore_power_ok(uint32_t* value)
{
	*value = atomic_get(&shore_power_ok);
}

// CAN bus ok flag
void ekset_can_bus_ok(const uint32_t value)
{
	atomic_set(&can_bus_ok, (atomic_val_t)value);
}

void ekget_can_bus_ok(uint32_t* value)
{
	*value = atomic_get(&can_bus_ok);
}

// Ready state flag
void ekset_ready_state(const uint32_t value)
{
	atomic_set(&ready_state, (atomic_val_t)value);
}

void ekget_ready_state(uint32_t* value)
{
	*value = atomic_get(&ready_state);
}

//----------------------------------------------------------------------
// - SECTION - ERS diagnostics
//----------------------------------------------------------------------

/**
 * @brief ERS firmware sends periodic diagnostic and state info over the debug
 *   UART.  This trio of routines controls these diagnostics in some simple
 *   frequency, enable and disable ways.
 */

void keeper_set_diag_periodic(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)true);
}

void keeper_clear_diag_periodic(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)false);
}

void keeper_get_diag_mode(uint32_t* value)
{
	*value = atomic_get(&ers_diag_flag_fs);
}

//----------------------------------------------------------------------
// - SECTION - initialization
//----------------------------------------------------------------------

int32_t keeper_set_hall_sensor_default_limits(void)
{
	int32_t rc = set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_UNDER, HALL_LIMIT_V_UNDER_S1);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_INACTIVE, HALL_LIMIT_V_INACTIVE_S1);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_BETWEEN, HALL_LIMIT_V_BETWEEN_S1);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_ACTIVE, HALL_LIMIT_V_ACTIVE_S1);

	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_UNDER, HALL_LIMIT_V_UNDER_S2);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_INACTIVE, HALL_LIMIT_V_INACTIVE_S2);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_BETWEEN, HALL_LIMIT_V_BETWEEN_S2);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_ACTIVE, HALL_LIMIT_V_ACTIVE_S2);

	return rc;
}

static int32_t initialize_system_state_vars(void)
{
	uint32_t count = 0;
	int32_t rc = 0;

	// TODO [ ] Create a clear or shared symbol to hold the time-wise interval for ERS
	//          firmware to determine and update lock ring position.
	atomic_set(&ring_pos_interval, (atomic_val_t)100);

	summary_state.ring_position = ATOMIC_INIT(RING_POS_UNKNOWN);
	summary_state.battery_voltage =  ATOMIC_INIT(0); 
	summary_state.battery_ok = ATOMIC_INIT(0); 
	summary_state.shore_power_ok = ATOMIC_INIT(0);
	summary_state.can_bus_ok = ATOMIC_INIT(0);
	summary_state.ready_flag = ATOMIC_INIT(0);

	rc = keeper_set_hall_sensor_default_limits();

	if (rc != 0)
	{
		LOG_ERR("Failed to set one or more of Hall limit default values, error %d", rc);
		rc = -EINVAL;
	}

	// Retrieve (read from flash) ring lock event count and unlock event count:

	rc = retrieve_ers_setting(KEY_NAME_LOCK_COUNT, (void *)count, sizeof(count));
	if (rc != 0) {
		LOG_ERR("Failed to retrieve ring lock event count, err %d", rc);
		set_ring_lock_event_count(RING_LOCK_EVENT_STARTING_COUNT);
		rc = store_ers_setting(KEY_NAME_LOCK_COUNT, (void *)count, sizeof(count));
		if (rc != 0) {
			LOG_ERR("Failed to write ring lock count, err %d", rc);
		}
	} else {
		set_ring_lock_event_count(count);
	}

	rc = retrieve_ers_setting(KEY_NAME_UNLOCK_COUNT, (void *)count, sizeof(count));
	if (rc != 0) {
		LOG_ERR("Failed to retrieve ring unlock events count, err %d", rc);
		set_ring_unlock_event_count(RING_UNLOCK_EVENT_STARTING_COUNT);
		rc = store_ers_setting(KEY_NAME_UNLOCK_COUNT, (void *)count, sizeof(count));
		if (rc != 0) {
			LOG_ERR("Failed to write ring unlock count, err %d", rc);
		}
	} else {
		set_ring_unlock_event_count(count);
	}

	return rc;
}

int32_t keeper_init(void)
{
	k_mutex_init(&hall_sensors_mtx);
	initialize_system_state_vars();
	keeper_initialized_fs = true;
	return 0;
}
