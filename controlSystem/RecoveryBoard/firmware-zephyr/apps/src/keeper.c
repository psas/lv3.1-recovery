/**
 * @file
 * @brief ERS keeper module, to hold and to share run time state.
 */

#include "arbiter.h"
#include "hall-and-ring.h"
#include "keeper.h"
#include "status-led.h"
#include "settings-ers.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(keeper, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

// There happen to be eight Hall sensor limit values to store, and to retrieve
// from flash memory.  For "within this file" house keeping, add symbols to
// store bit-wise left shift values.  These are used to track store and retrieve
// errors, to allow for attempting further retrieve ops even when some fail.

enum keeper_store_value_result {
KEEPER_OP_1_SHIFT = 0,
KEEPER_OP_2_SHIFT,
KEEPER_OP_3_SHIFT,
KEEPER_OP_4_SHIFT,

KEEPER_OP_5_SHIFT,
KEEPER_OP_6_SHIFT,
KEEPER_OP_7_SHIFT,
KEEPER_OP_8_SHIFT,

COUNT_OF_STORE_RESULTS,
};

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
static atomic_t motor_isense_ma = ATOMIC_INIT(0);
static atomic_t dac_setting_ring_motor = ATOMIC_INIT(0);

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

#if 1
// TODO [ ] Consider factoring summary state variables into a structure,
//  this may improve code readability and mainenance:

// static atomic_t ring_status = ATOMIC_INIT(0);
// QUESTION - put battery voltage in struct of ERS states?
static atomic_t batt_ok = ATOMIC_INIT(0);
static atomic_t shore_power_ok = ATOMIC_INIT(0);
static atomic_t can_bus_ok = ATOMIC_INIT(0);
static atomic_t rocket_ready = ATOMIC_INIT(0);
#endif // 0

// TODO [ ] create public API getter for CAN module to access ERS summary state.

struct ers_summary_state {
	atomic_t ring_position;
	atomic_t battery_voltage;
	atomic_t battery_ok;
	atomic_t shore_power_ok;
	atomic_t can_bus_ok;
	atomic_t ready_flag;
};

static struct ers_summary_state summary_state_fs;

// Support run time toggling of diagnostics which share UART with Zephyr shell:
static atomic_t ers_diag_flag_fs = ATOMIC_INIT(0);

enum data_operation {
	KEEPER_OP_SET,
	KEEPER_OP_GET,
	KEEPER_OP_STORE,
	KEEPER_OP_RETRIEVE,
	KEEPER_LAST_OP,
};

#define OP_NAME_LENGTH 9

static char op_name_fs[][OP_NAME_LENGTH] = {
	"set\0", "get\0", "store\0", "retrieve\0",
};

static char undef_string_fs[] = { "unknown_op" };

// Provide a mutex to assure that both Hall sensors are updated without anyone
// reading their latest values in the middle of this pair of updates:
struct k_mutex hall_sensors_mtx;

// Flag to indiciate that this module is initialized:
static bool keeper_initialized_fs = false;

//----------------------------------------------------------------------
// - SECTION - prototypes
//----------------------------------------------------------------------

/**
 * @brief Routine to set a given Hall sensor limit, a cutoff value
 *   measured in ADC counts, for each hall sensor in an ERS board.
 *
 * @retval 0 when sensor id, limit id in bounds.
 *
 * @retval -EINVAL otherwise.
 */

static int32_t set_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
				     const enum hall_sensor_named_limits limit_idx,
				     const uint32_t val);

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (1) battery
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Battery reading in ADC counts
void keeper_set_batt_read(const uint32_t value)
{
	atomic_set(&batt_read, (atomic_val_t)value);
}

void keeper_get_batt_read(uint32_t* value)
{
	*value = atomic_get(&batt_read);
}

// Battery reading in millivolts
void keeper_set_batt_read_mv(const uint32_t value)
{
	atomic_set(&batt_read_mv, (atomic_val_t)value);
}

void keeper_get_batt_read_mv(uint32_t* value)
{
	*value = atomic_get(&batt_read_mv);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (2) digital inputs
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void keeper_set_iso_drogue(const uint32_t value)
{
	atomic_set(&iso_drogue, (atomic_val_t)value);
}

void keeper_set_iso_main(const uint32_t value)
{
	atomic_set(&iso_main, (atomic_val_t)value);
}

void keeper_set_not_umb_on(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

void keeper_get_iso_drogue(uint32_t* value)
{
	*value = atomic_get(&iso_drogue);
}

void keeper_get_iso_main(uint32_t* value)
{
	*value = atomic_get(&iso_main);
}

void keeper_get_not_umb_on(uint32_t* value)
{
	*value = atomic_get(&not_umb_on);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (2 1/2) analog inputs not categorized
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int32_t keeper_set_adc_value(const enum ers_adc_values idx, const uint32_t val)
{
	switch (idx)
	{
        case ADC_READING_BATT_READ:
		keeper_set_batt_read(val);
		break;
        case ADC_READING_MOTOR_ISENSE:
		keeper_set_motor_isense(val);
		break;
	case ADC_READING_HALL_1:
		keeper_set_hall_1(val);
		break;
        case ADC_READING_HALL_2:
		keeper_set_hall_2(val);
		break;
	default:
		LOG_ERR("Asked to store value for undefined ADC channel %d", idx);
		return -EINVAL;
	}

	return 0;
}

int32_t keeper_set_adc_value_in_mv(const enum ers_adc_values_in_mv idx, const uint32_t val)
{
	switch (idx)
	{
        case ADC_READING_BATT_READ_MV:
		keeper_set_batt_read_mv(val);
		break;
        case ADC_READING_MOTOR_ISENSE_MV:
		keeper_set_motor_isense_ma(val);
		break;
        case ADC_READING_HALL_1_MV:
		keeper_set_hall_1_mv(val);
		break;
        case ADC_READING_HALL_2_MV:
		keeper_set_hall_2_mv(val);
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

int32_t keeper_cmd_set_limit_v_under(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *str, *endptr;
	enum hall_sensor_instances sensor_idx;
	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		rc = -EINVAL;
		goto done;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
        if (*endptr != '\0') {
		shell_fprintf(shell, SHELL_WARNING, "Parsed non-numeric "
				"characters after number: '%s'\n", endptr);
		rc = -EINVAL;
		goto done;
	}

	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'v_under' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_UNDER, value);

done:
	return rc;
}

int32_t keeper_cmd_set_limit_inactive(const struct shell *shell, size_t argc, char **argv)
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

int32_t keeper_cmd_set_limit_between(const struct shell *shell, size_t argc, char **argv)
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

int32_t keeper_cmd_set_limit_active(const struct shell *shell, size_t argc, char **argv)
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

/**
 * @brief Helper function to return the name of a data operation, used in
 *  error messages.
 */

static char *op_name(const enum data_operation op)
{
	if (op < KEEPER_LAST_OP) {
		return op_name_fs[op];
	} else {
		return undef_string_fs;
	}
}

/**
 * @brief Helper function to update bit-wise error flags, to track the outcome
 *  of data store operations.
 *
 * @param shift the power by which to raise the value 1.
 * @param top_count the total number of store operations to be tracked.
 * @param rc the return code of the API to store a datum.
 *
 * @return the value of 2 raised to the power of 'shift'.
 */

static uint32_t track_err(const enum data_operation op, const uint32_t shift,
			const uint32_t top_count, const int32_t rc)
{
	LOG_ERR("Failed to %s Hall limit %d of %d, err %d", op_name(op),
	       	shift + 1, top_count, rc);
	return (1 << shift);
}

int32_t keeper_cmd_store_hall_limits(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	// Declare and define an array of Hall sensor limit values,
	// which will be used for each Hall sensor in the system:
	uint32_t hall_limit[HALL_SENSOR_LIMIT_COUNT] = { 0 };

	// Track any errors in get and store operations, so that we can at
	// least partially succeed if some values fail in these operations:
	int32_t get_errors = 0;
	int32_t store_errors = 0;
	enum data_operation op = KEEPER_OP_GET;
	int32_t rc = 0;

#define ERR_FLAG_BIT_SHIFT (j + HALL_SENSOR_LIMIT_COUNT * i)

	for (uint32_t i = 0; i < HALL_SENSOR_COUNT; i++) {
		for (uint32_t j = 0; j < HALL_SENSOR_LIMIT_COUNT; j++) {
			rc = keeper_get_hall_sensor_limit(i, j, &hall_limit[j]);

			// LOG_INF("- DEV - for Hall %d limit %d, error flag bit shift is %d", i, j, ERR_FLAG_BIT_SHIFT);

			if (rc < 0) {
				op = KEEPER_OP_GET;
				get_errors |= track_err(op, ERR_FLAG_BIT_SHIFT,
					       	COUNT_OF_STORE_RESULTS, rc);
				LOG_ERR("Failed to get Hall %d limit %d, err %d", i, j, rc);
				// When get op fails, do not attempt to store anything:
				continue;
			}
			rc = ers_settings_store_hall_limit(i, j,
					       		(const void *)hall_limit[j],
							sizeof(hall_limit[j]));
			if (rc < 0) {
				op = KEEPER_OP_STORE;
				store_errors |= track_err(op, ERR_FLAG_BIT_SHIFT,
					       	COUNT_OF_STORE_RESULTS, rc);
			}
		}
	}

	if (get_errors) {
		shell_fprintf(shell, SHELL_ERROR, "Failed to store some Hall sensor limits,\n");
		shell_fprintf(shell, SHELL_ERROR, "bit-wise errors in hexadecimal are 0x%02X,\n",
				get_errors);
	}

	if (store_errors) {
		shell_fprintf(shell, SHELL_ERROR, "Failed to store some Hall sensor limits,\n");
		shell_fprintf(shell, SHELL_ERROR, "bit-wise errors in hexadecimal are 0x%02X,\n",
				store_errors);
		rc = store_errors;
	} else {
		shell_fprintf(shell, SHELL_NORMAL, "Hall sensor limit values stored to flash.\n");
	}

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

int32_t keeper_cmd_retrieve_hall_limits(const struct shell *shell, size_t argc, char **argv)
{
	int32_t rc1 = 0;
	int32_t rc2 = 0;

	rc1 = keeper_retrieve_hall_1_limits();
	if (rc1 < 0) {
		LOG_ERR("Failed to retrieve Hall sensor 1 limits, err %d", rc1);
	}

	rc2 = keeper_retrieve_hall_2_limits();
	if (rc2 < 0) {
		LOG_ERR("Failed to retrieve Hall sensor 2 limits, err %d", rc2);
	}

	if (rc1 || rc2) {
		shell_fprintf(shell, SHELL_NORMAL, "WARNING: Some Hall sensor limits "
				"not retrieved.  Enter 'hall show' to see them\n");
	} else {
		shell_fprintf(shell, SHELL_NORMAL, "Hall sensor limits retrieved.  "
				"Enter 'hall show' to see them\n");
	}

	if (rc1 || rc2) {
		LOG_ERR("Some or all Hall sensor limits failed to be retrieved.");
		LOG_ERR("sensor 1 limits return code = %d, sensor 2 code = %d",
				rc1, rc2);
		LOG_ERR("Returning negative EIO to indicate I/O error with flash memory.");
		return -EIO;
	} else {
		return 0;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (3) locking ring
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Getters

void keeper_get_hall_1(uint32_t* value)
{
	*value = atomic_get(&hall_1);
}

void keeper_get_hall_2(uint32_t* value)
{
	*value = atomic_get(&hall_2);
}

void keeper_get_hall_1_mv(uint32_t* value)
{
	*value = atomic_get(&hall_1_mv);
}

void keeper_get_hall_2_mv(uint32_t* value)
{
	*value = atomic_get(&hall_2_mv);
}

// Setters

void keeper_set_hall_1(const uint32_t value)
{
	atomic_set(&hall_1, (atomic_val_t)value);
}

void keeper_set_hall_2(const uint32_t value)
{
	atomic_set(&hall_2, (atomic_val_t)value);
}

void keeper_set_hall_1_mv(const uint32_t value)
{
	atomic_set(&hall_1_mv, (atomic_val_t)value);
}

void keeper_set_hall_2_mv(const uint32_t value)
{
	atomic_set(&hall_2_mv, (atomic_val_t)value);
}

/**
 * @brief Function to store both Hall sensor readings with mutual exclusion
 *   to assure these values are read only when both are up to date.
 */

int32_t keeper_set_both_hall_sensors(const uint32_t value_1, const uint32_t value_2)
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

	keeper_set_hall_1(value_1);
	keeper_set_hall_2(value_2);

	k_mutex_unlock(&hall_sensors_mtx);
	if (rc != 0)
	{
		LOG_ERR("Failed to lock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	return 0;
}

int32_t keeper_get_both_hall_sensors(uint32_t *value_1, uint32_t *value_2)
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

	keeper_get_hall_1_mv(value_1);
	keeper_get_hall_2_mv(value_2);

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

static int32_t set_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
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

int32_t keeper_get_hall_sensor_limit(const enum hall_sensor_instances sensor_idx,
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

void keeper_set_ring_pos_detection_interval(const uint32_t timeout_ms)
{
	atomic_set(&ring_pos_interval, (atomic_val_t)timeout_ms);
}

void keeper_get_ring_pos_detection_interval(uint32_t *timeout_ms)
{
	*timeout_ms = atomic_get(&ring_pos_interval);
}

void keeper_set_detected_ring_position(const enum lock_ring_position ring_pos)
{
	atomic_set(&summary_state_fs.ring_position, (atomic_val_t)ring_pos);
}

void keeper_get_detected_ring_position(enum lock_ring_position *ring_pos)
{
	*ring_pos = atomic_get(&summary_state_fs.ring_position);
}

// Parachute section ring lock and unlock events

void keeper_set_lock_event_count(const uint32_t count)
{
	atomic_set(&ring_lock_events, (atomic_val_t)count);
}

void keeper_get_unlock_event_count(uint32_t *count)
{
	*count = atomic_get(&ring_unlock_events);
}

void keeper_set_unlock_event_count(const uint32_t count)
{
	atomic_set(&ring_unlock_events, (atomic_val_t)count);
}

void keeper_get_lock_event_count(uint32_t *count)
{
	*count = atomic_get(&ring_lock_events);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (4) motor
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// motor current reading in ADC counts
void keeper_set_motor_isense(const uint32_t value)
{
	atomic_set(&motor_isense, (atomic_val_t)value);
}

void keeper_get_motor_isense(uint32_t* value)
{
	*value = atomic_get(&motor_isense);
}

// motor current reading in milliamps
void keeper_set_motor_isense_ma(const uint32_t value)
{
	atomic_set(&motor_isense_ma, (atomic_val_t)value);
}

void keeper_get_motor_isense_ma(uint32_t* value)
{
	*value = atomic_get(&motor_isense_ma);
}

// motor digitnal status signal out
// TODO [ ] Do we really need to store "not motor faila", or is it used
//          immediately and volatile in a practical sense?
void keeper_set_not_motor_faila(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

void keeper_get_not_motor_faila(uint32_t* value)
{
	*value = atomic_get(&not_motor_faila);
}

// DAC setting to use for ring lock and ring unlock motor actuations
void keeper_set_DAC_val_for_ring_motor(const uint32_t value)
{
	atomic_set(&dac_setting_ring_motor, (atomic_val_t)value);
}

void keeper_get_DAC_val_for_ring_motor(uint32_t *value)
{
	*value = atomic_get(&dac_setting_ring_motor);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (6) ERS summary state data
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Lock ring status
void keeper_set_ring_status(const enum lock_ring_state value)
{
	// atomic_set(&ring_status, (atomic_val_t)value);
	atomic_set(&summary_state_fs.ring_position, (atomic_val_t)value);
}

void keeper_get_ring_status(enum lock_ring_state *value)
{
	// *value = atomic_get(&ring_status);
	*value = atomic_get(&summary_state_fs.ring_position);
}

// Battery voltage
void keeper_set_battery_decivolts(const uint32_t value)
{
	atomic_set(&batt_read_dv, (atomic_val_t)value);
}

void keeper_get_battery_decivolts(uint32_t *value)
{
	*value = atomic_get(&batt_read_dv);
}

// Battery ok flag
void keeper_set_batt_ok(const uint32_t value)
{
	atomic_set(&batt_ok, (atomic_val_t)value);
}

void keeper_get_batt_ok(uint32_t* value)
{
	*value = atomic_get(&batt_ok);
}

// Shore power ok flag
void keeper_set_shore_power_ok(const uint32_t value)
{
	atomic_set(&shore_power_ok, (atomic_val_t)value);
}

void keeper_get_shore_power_ok(uint32_t* value)
{
	*value = atomic_get(&shore_power_ok);
}

// CAN bus ok flag
void keeper_set_can_bus_ok(const uint32_t value)
{
	atomic_set(&can_bus_ok, (atomic_val_t)value);
}

void keeper_get_can_bus_ok(uint32_t* value)
{
	*value = atomic_get(&can_bus_ok);
}

// Rocket ready state flag
void keeper_set_ready_state(const uint32_t value)
{
	atomic_set(&rocket_ready, (atomic_val_t)value);
}

void keeper_get_ready_state(uint32_t* value)
{
	*value = atomic_get(&rocket_ready);
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

	atomic_set(&ring_pos_interval, (atomic_val_t)CONFIG_ARBITER_LOOP_SLEEP_PER_MS);

	summary_state_fs.ring_position = ATOMIC_INIT(RING_POS_UNKNOWN);
	summary_state_fs.battery_voltage =  ATOMIC_INIT(0); 
	summary_state_fs.battery_ok = ATOMIC_INIT(0); 
	summary_state_fs.shore_power_ok = ATOMIC_INIT(0);
	summary_state_fs.can_bus_ok = ATOMIC_INIT(0);
	summary_state_fs.ready_flag = ATOMIC_INIT(0);

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
		keeper_set_lock_event_count(RING_LOCK_EVENT_STARTING_COUNT);
		rc = store_ers_setting(KEY_NAME_LOCK_COUNT, (void *)count, sizeof(count));
		if (rc != 0) {
			LOG_ERR("Failed to write ring lock count, err %d", rc);
		}
	} else {
		keeper_set_lock_event_count(count);
	}

	rc = retrieve_ers_setting(KEY_NAME_UNLOCK_COUNT, (void *)count, sizeof(count));
	if (rc != 0) {
		LOG_ERR("Failed to retrieve ring unlock events count, err %d", rc);
		keeper_set_unlock_event_count(RING_UNLOCK_EVENT_STARTING_COUNT);
		rc = store_ers_setting(KEY_NAME_UNLOCK_COUNT, (void *)count, sizeof(count));
		if (rc != 0) {
			LOG_ERR("Failed to write ring unlock count, err %d", rc);
		}
	} else {
		keeper_set_unlock_event_count(count);
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
