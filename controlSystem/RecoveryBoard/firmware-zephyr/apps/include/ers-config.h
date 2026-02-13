#ifndef ERS_CONFIG_DEFAULTS
#define ERS_CONFIG_DEFAULTS

/**
 * @brief Header file to capture ERS default board configuration and firmware
 *  settings.
 */

/**
 * @note Following tables shows Hall sensor states in left-most column and in
 *  top-most row.  States are determined by comparing Hall sensor readings
 *  (measured in ADC counts) against empirically determined limits.
 */

/*
        Hall2 |
 Hall1        |   Over     Active    Between   Inactive    Under
--------------+------------------------------------------------------
Over          |   error    locked    betwen    unlocked    error
Active        |  unlocked   error    between   UNLOCKED   unlocked
Between       |   between  between   BETWEEN    between    between
Unactive      |   locked    LOCKED   between     error     locked
Under         |   error     locked   between   unlocked     error
*/

/**
 * (C1) Hall sensor default limit values
 */

#define HALL_AND_SENSORS_DEFAULT_LIMIT_VALUES \
HALL_SENSOR_INST(1, 750, 920, 1600, 3100) \
HALL_SENSOR_INST(2, 800, 950, 2600, 3100)

/**
 * (C2) Hall sensor instance enumeration
 */

#define HALL_SENSOR_INST(sensor_idx, limit1, limit2, limit3, limit4) HALL_SENSOR_##sensor_idx,
enum hall_sensor_instances {
HALL_AND_SENSORS_DEFAULT_LIMIT_VALUES
HALL_SENSOR_COUNT
};
#undef HALL_SENSOR_INST

/**
 * (C3) Hall sensor state names
 */

#define  LIMIT_NAME_1  HALL_LIMIT_V_UNDER
#define  LIMIT_NAME_2  HALL_LIMIT_V_INACTIVE
#define  LIMIT_NAME_3  HALL_LIMIT_V_BETWEEN
#define  LIMIT_NAME_4  HALL_LIMIT_V_ACTIVE

#define  LIMIT_VARNAME_1  hall_limit_v_under
#define  LIMIT_VARNAME_2  hall_limit_v_inactive
#define  LIMIT_VARNAME_3  hall_limit_v_between
#define  LIMIT_VARNAME_4  hall_limit_v_active

// Macros to construct keynames for Hall sensor cutoff values stored in flash
#define KEYNAME_PREFIX setting_

#if 0
#define SETTING_KEYNAME_HLIMIT_1 KEYNAME_PREFIX ## LIMIT_VARNAME_1
#define SETTING_KEYNAME_HLIMIT_2 KEYNAME_PREFIX ## LIMIT_VARNAME_2
#define SETTING_KEYNAME_HLIMIT_3 KEYNAME_PREFIX ## LIMIT_VARNAME_3
#define SETTING_KEYNAME_HLIMIT_4 KEYNAME_PREFIX ## LIMIT_VARNAME_4
#endif

// A test macro:
#define SETTING_KEYNAME_S2_HL1 STRINGIFY(KEYNAME_PREFIX) "s2" "_" STRINGIFY(LIMIT_VARNAME_1)

// Keynames for Hall sensor 1
#define SETTING_KEYNAME_S1_HLIMIT_1 STRINGIFY(KEYNAME_PREFIX) "s1" "_" STRINGIFY(LIMIT_VARNAME_1)
#define SETTING_KEYNAME_S1_HLIMIT_2 STRINGIFY(KEYNAME_PREFIX) "s1" "_" STRINGIFY(LIMIT_VARNAME_2)
#define SETTING_KEYNAME_S1_HLIMIT_3 STRINGIFY(KEYNAME_PREFIX) "s1" "_" STRINGIFY(LIMIT_VARNAME_3)
#define SETTING_KEYNAME_S1_HLIMIT_4 STRINGIFY(KEYNAME_PREFIX) "s1" "_" STRINGIFY(LIMIT_VARNAME_4)

// Keynames for Hall sensor 2
#define SETTING_KEYNAME_S2_HLIMIT_1 STRINGIFY(KEYNAME_PREFIX) "s2" "_" STRINGIFY(LIMIT_VARNAME_1)
#define SETTING_KEYNAME_S2_HLIMIT_2 STRINGIFY(KEYNAME_PREFIX) "s2" "_" STRINGIFY(LIMIT_VARNAME_2)
#define SETTING_KEYNAME_S2_HLIMIT_3 STRINGIFY(KEYNAME_PREFIX) "s2" "_" STRINGIFY(LIMIT_VARNAME_3)
#define SETTING_KEYNAME_S2_HLIMIT_4 STRINGIFY(KEYNAME_PREFIX) "s2" "_" STRINGIFY(LIMIT_VARNAME_4)

// Tokens to be stringified for Hall sensor state names
#define  STATE_NAME_1  HALL_STATE_V_UNDER
#define  STATE_NAME_2  HALL_STATE_V_INACTIVE
#define  STATE_NAME_3  HALL_STATE_V_BETWEEN
#define  STATE_NAME_4  HALL_STATE_V_ACTIVE
#define  STATE_NAME_5  HALL_STATE_V_OVER
#define  STATE_NAME_6  HALL_STATE_UNKNOWN

/**
 * (C4) Enumeration of Hall sensor limits (named identifiers):
 */

enum hall_sensor_named_limits {
LIMIT_NAME_1,
LIMIT_NAME_2,
LIMIT_NAME_3,
LIMIT_NAME_4,
HALL_SENSOR_LIMIT_COUNT
};

#define CONCAT_A_B_C(prefix, infix, name) prefix ## infix ## name
#define INTERMEDIATE_A_B_C(prefix, infix, name) CONCAT_A_B_C(prefix, infix, name)

#define LIMIT_NAME(prefix, infix, name) CONCAT_A_B_C(prefix, infix, name)

#define HALL_SENSOR_INST(idx, limit1, limit2, limit3, limit4) \
HALL_SENSOR_CONSTRUCT(LIMIT_NAME(LIMIT_NAME_1, _S, idx), LIMIT_NAME(LIMIT_VARNAME_1, _S, idx), limit1) \
HALL_SENSOR_CONSTRUCT(LIMIT_NAME(LIMIT_NAME_2, _S, idx), LIMIT_NAME(LIMIT_VARNAME_2, _S, idx), limit2) \
HALL_SENSOR_CONSTRUCT(LIMIT_NAME(LIMIT_NAME_3, _S, idx), LIMIT_NAME(LIMIT_VARNAME_3, _S, idx), limit3) \
HALL_SENSOR_CONSTRUCT(LIMIT_NAME(LIMIT_NAME_4, _S, idx), LIMIT_NAME(LIMIT_VARNAME_4, _S, idx), limit4)

/**
 * (C5) Hall sensor limit variable names
 */

#define HALL_SENSOR_CONSTRUCT(limit_name, limit_var_name, limit_value) \
	limit_name = limit_value,
enum hall_sensor_limit_defaults {
	HALL_AND_SENSORS_DEFAULT_LIMIT_VALUES
};
#undef HALL_SENSOR_CONSTRUCT

/**
 * (C6) Hall sensor named states
 */

enum hall_sensor_state_ids {
STATE_NAME_1,
STATE_NAME_2,
STATE_NAME_3,
STATE_NAME_4,
STATE_NAME_5,
STATE_NAME_6,
};

#undef HALL_SENSOR_CONSTRUCT

#define BASE_10 10

#endif // ERS_CONFIG_DEFAULTS
