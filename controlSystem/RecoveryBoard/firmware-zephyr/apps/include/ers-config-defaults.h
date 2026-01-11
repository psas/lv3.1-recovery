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

// - X MACRO 1 - Hall sensor default empirical limits per sensor

// Define an outer macro which encapsulates empirically determined Hall sensor limit values:

#define HALL_SENSORS_DEFAULT_LIMIT_VALUES \
HALL_SENSOR_INST(1, 750, 920, 1600, 3100) \
HALL_SENSOR_INST(2, 800, 950, 2600, 3100)

// - X MACRO 2 -

// (1) limit identifier
// (2) per sensor limit identifier
// (3) limit value
// (4) settings keyname for limit value
// (5) Hall output state

// clang-format off
#define HALL_SENSOR_INST(idx, limit1, limit2, limit3, limit4) \
HALL_SENSOR_CONSTRUCT(HL_V_UNDER,   HALL_##idx##_LIMIT_V_UNDER, limit1, "hall-##idx##-limit-v-under",  HALL_OUTPUT_UNDER_VOLTAGE) \
HALL_SENSOR_CONSTRUCT(HL_INACTIVE, HALL_##idx##_LIMIT_INACTIVE, limit2, "hall-##idx##-limit-inactive", HALL_OUTPUT_INACTIVE)      \
HALL_SENSOR_CONSTRUCT(HL_BETWEEN,   HALL_##idx##_LIMIT_BETWEEN, limit3, "hall-##idx##-limit-between",  HALL_OUTPUT_BETWEEN)       \
HALL_SENSOR_CONSTRUCT(HL_ACTIVE,     HALL_##idx##_LIMIT_ACTIVE, limit4, "hall-##idx##-limit-active",   HALL_OUTPUT_ACTIVE)        \
//                    ^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^
//                     |             |                           |       |                             |
//                    limit ids     per sensor limit ids        limit   settings keyname              Hall state identifiers
//                                                              values
// clang-format on

// Create identifiers for Hall sensor limits:

#define HALL_SENSOR_CONSTRUCT(limit, per_sensor_limit, value, keyname, hall_state) limit,

enum hall_sensor_limit_ids {
HALL_SENSOR_INST(0, 0, 0, 0, 0)
HALL_SENSOR_LIMIT_COUNT
};

#undef HALL_SENSOR_CONSTRUCT

// Create symbols for Hall sensor default limit values:
// (Values determined empirically from lock ring assembly measurement exercises.)

#define HALL_SENSOR_CONSTRUCT(limit, per_sensor_limit, value, keyname, hall_state) per_sensor_limit = value,

enum hall_sensor_limit_defaults {
HALL_SENSORS_DEFAULT_LIMIT_VALUES
};

#undef HALL_SENSOR_CONSTRUCT

// Create Hall sensor state ids:

#define HALL_SENSOR_CONSTRUCT(limit, per_sensor_limit, value, keyname, hall_state) hall_state,

enum hall_sensor_state_ids {
HALL_SENSOR_INST(0, 0, 0, 0, 0)
HALL_OUTPUT_OVER_VOLTAGE,
HALL_OUTPUT_UNKNOWN
};

#undef HALL_SENSOR_CONSTRUCT

#endif // ERS_CONFIG_DEFAULTS
