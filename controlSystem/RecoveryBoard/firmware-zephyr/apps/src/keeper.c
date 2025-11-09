/**
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(keeper, LOG_LEVEL_INF);

#include <arbiter.h>
#include <ers-config-defaults.h>
#include <keeper.h>

/**
 * @brief ERS "keeper" module, acts like a bulletin board to hold shared data
 *   across the app.
 *
 * @note ERS keeper module tracks and shares most ERS board condition and state
 *   information.  State info is organized in this module in the following
 *   sets of config settings, readings and "programmatic" conditions.  These are
 *   collectively named "data groups":
 *
 * (1) battery (for recovery system)
 *    +  voltage reading
 *    +  battery "ok" minimum limit
 *
 * (2) simple digital inputs
 *    +  umbilical (shore) power connected
 *    +  ISO_DROGUE input on "Sender" ERS board only
 *    +  ISO_MAIN input on "Sender" ERS board only
 *
 * (2 1/2) analog input not categorized
 *
 * (3) locking ring
 *    +  Hall sensors 1
 *       o  reading
 *       o  v_under ADC count limit
 *       o  inactive ADC count limit
 *       o  between ADC count limit
 *       o  active ADC count limit
 *    +  Hall sensors 2
 *       o  reading
 *       o  v_under ADC count limit
 *       o  inactive ADC count limit
 *       o  between ADC count limit
 *       o  active ADC count limit
 *    +  ring state, present physical position: locked, between, unlocked, unknown
 *    +  ring position detection interval
 *
 * (4) motor related (motor for lock ring)
 *    +  Motor
 *       o  MOTOR_ISENSE analog input for current reading
 *       o  NOT_MOTOR_FAILA digital input
 *       o  NOT_MOTOR_PS output to enable H-bridge
 *       o  DAC output to control current to motor
 *       o  DEPLOY1 one of two H-bridge control signals
 *       o  DEPLOY2 two of two H-bridge control signals
 *
 * (5) CAN bus related
 *
 * (6) ERS summary state data
 *    +  ring_status
 *    +  battery_voltage (in decivolts)
 *    +  battery_ok
 *    +  shore_power_ok
 *    +  can_bus_ok
 *    +  ready_state
 *
 * (7) ERS diagnostics
 */

/*
------------------------------------------------------------------------
Summary of known ERS inputs 2025-09-25, subject to be updated:

Digital:

[x] !UMB_ON (PA8 = GPIO input)
[x] ISO_DROGUE (PA5 = GPIO input)      . . . "Sender" ERS board only
[x] ISO_MAIN  (PA6 = GPIO input)       . . . "Sender" ERS board only
[ ] !MOTOR_FAILA (PB7 = GPIO input)

Analog:

[x] BATT_READ (PB0 = analog input ADC_IN8)
[x] MOTOR_ISENSE (PB1 = Analog input ADC_IN9)
[x] HALL1 (PA0 = Analog input ADC_IN0)
[x] HALL2 (PA1 = Analog input ADC_IN1)

Message based inputs:

[ ] CAN module   . . . given ERS board/firmware variant will keep track
                       of one or more CAN messages, through state
                       variables.
------------------------------------------------------------------------
*/ 

// some GPIO inputs, effectively Boolean

/**
 * @defgroup digital_inputs
 */

static atomic_t iso_drogue = ATOMIC_INIT(0);
static atomic_t iso_main = ATOMIC_INIT(0);
static atomic_t not_umb_on = ATOMIC_INIT(0);

// analog inputs, typically 12-bit or 16-bit values

/**
 * @defgroup battery
 */

static atomic_t batt_read = ATOMIC_INIT(0);
static atomic_t batt_read_mv = ATOMIC_INIT(0);
// TODO [ ] Refactor battery millivolt to decivolt conversion to occur
//   after calls to get batter voltage:
static atomic_t batt_read_dv = ATOMIC_INIT(0);

/**
 * @defgroup motor_related
 */

static atomic_t not_motor_faila = ATOMIC_INIT(0);

static atomic_t motor_isense = ATOMIC_INIT(0);
static atomic_t hall_1 = ATOMIC_INIT(0);
static atomic_t hall_2 = ATOMIC_INIT(0);

static atomic_t motor_isense_mv = ATOMIC_INIT(0);
static atomic_t hall_1_mv = ATOMIC_INIT(0);
static atomic_t hall_2_mv = ATOMIC_INIT(0);

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

struct hall_sensor_limits {
	atomic_t v_under;
	atomic_t inactive;
	atomic_t between;
	atomic_t active;
};

static struct hall_sensor_limits hall_sensor_fs[HALL_SENSOR_COUNT];

static atomic_t ring_pos_interval = ATOMIC_INIT(0);

/**
 * @defgroup system_state
 */

// Off-chip peripherals and system statae
//
// [ ] ring status    . . . one of 2 = locked, 1 = between, 0 = unlocked
// [x] batt_voltage   . . . among analog inputs
// [ ] batt_ok        . . . a threshold based state
// [ ] shore_power_ok . . . 1 = shore power detected, 0 = no shore power
// [x] can_bus_ok     . . . telemetrum heartbeat received within last two seconds
// [ ] ready_state    . . . true when (1) battery ok (2) ring locked (3) CAN bus ok
// [ ] reserved
// [ ] reserved

// TODO [ ] remove these individual file scoped variables in favor of
//          struct to organize them:

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

struct k_mutex hall_sensors_mtx;

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
#if 0
	static uint32_t call_count = 0;
	if ((call_count % 100) == 0)
	{
		LOG_INF("battery reading %u stored", value);
	}
	call_count++;
#endif
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

void ekget_not_unb_on(uint32_t* value)
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
		ekset_motor_isense_mv(val);
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

// "get" APIs for analog inputs

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

// Give ring state logic readings from same sample period:



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - DATA GROUP - (3) locking ring
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

	// k_mutex_lock(&hall_sensors_mtx, K_FOREVER);
	if (rc != 0)
	{
		LOG_ERR("Failed to lock mutex for \"store hall sensors values\", error %d", rc);
		return rc;
	}

	ekget_hall_1_mv(value_1);
	ekget_hall_2_mv(value_2);

	// k_mutex_unlock(&hall_sensors_mtx);
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

int32_t set_hall_sensor_limit(const enum hall_sensor_ids sensor_idx,
				const enum hall_sensor_limit_ids limit_idx,
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
	case HL_V_UNDER:
		atomic_set(&hall_sensor_fs[sensor_idx].v_under, value);
		break;
        case HL_INACTIVE:
		atomic_set(&hall_sensor_fs[sensor_idx].inactive, value);
		break;
        case HL_BETWEEN:
		atomic_set(&hall_sensor_fs[sensor_idx].between, value);
		break;
        case HL_ACTIVE:
		atomic_set(&hall_sensor_fs[sensor_idx].active, value);
		break;
	default:
	}

	return 0;
}

int32_t get_hall_sensor_limit(const enum hall_sensor_ids sensor_idx,
				const enum hall_sensor_limit_ids limit_idx,
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
	case HL_V_UNDER:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].v_under);
		break;
        case HL_INACTIVE:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].inactive);
		break;
        case HL_BETWEEN:
		*value = atomic_get(&hall_sensor_fs[sensor_idx].between);
		break;
        case HL_ACTIVE:
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
// TODO [ ] correct API name to reflect units of millamps not millivolts:
void ekset_motor_isense_mv(const uint32_t value)
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
 *   UART.  This pair of routines enables and disables this at run time.
 */

void ek_sys_diag_periodic(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)true);
}

void ek_sys_diag_quiet(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)false);
}

void ek_get_sys_diag_mode(uint32_t* value)
{
	*value = atomic_get(&ers_diag_flag_fs);
}

//----------------------------------------------------------------------
// - SECTION - initialization
//----------------------------------------------------------------------

int32_t set_hall_sensor_default_limits(void)
{
	int32_t rc = set_hall_sensor_limit(HALL_SENSOR_1, HL_V_UNDER, HALL_1_LIMIT_V_UNDER);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HL_INACTIVE, HALL_1_LIMIT_INACTIVE);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HL_BETWEEN, HALL_1_LIMIT_BETWEEN);
	rc |= set_hall_sensor_limit(HALL_SENSOR_1, HL_ACTIVE, HALL_1_LIMIT_ACTIVE);

	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HL_V_UNDER, HALL_2_LIMIT_V_UNDER);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HL_INACTIVE, HALL_2_LIMIT_INACTIVE);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HL_BETWEEN, HALL_2_LIMIT_BETWEEN);
	rc |= set_hall_sensor_limit(HALL_SENSOR_2, HL_ACTIVE, HALL_2_LIMIT_ACTIVE);

	return rc;
}

static int32_t initialize_system_state_vars(void)
{
	int32_t rc = 0;

	atomic_set(&ring_pos_interval, (atomic_val_t)RING_LOCKED);

	summary_state.ring_position = ATOMIC_INIT(RING_POSITION_UNKNOWN);
	summary_state.battery_voltage =  ATOMIC_INIT(0); 
	summary_state.battery_ok = ATOMIC_INIT(0); 
	summary_state.shore_power_ok = ATOMIC_INIT(0);
	summary_state.can_bus_ok = ATOMIC_INIT(0);
	summary_state.ready_flag = ATOMIC_INIT(0);

	rc = set_hall_sensor_default_limits();

	if (rc != 0)
	{
		LOG_ERR("Failed to set one or more of Hall limit default values, error %d", rc);
		rc = -EINVAL;
	}

	LOG_INF("M2");
	return rc;
}

int32_t ers_init_keeper(void)
{
	k_mutex_init(&hall_sensors_mtx);
	initialize_system_state_vars();
	keeper_initialized_fs = true;
	return 0;
}
