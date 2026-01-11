#ifndef ERS_CONFIG_DEFAULTS
#define ERS_CONFIG_DEFAULTS

/*
        Hall2 |
 Hall1        |   Over     Active    Between   Inactive    Under
--------------+------------------------------------------------------
Over          |   error    locked    betwen    unlocked    error
Active        |  unlocked   error    between   UNLOCKED   unlocked
Between       |   between  between   BETWEEN    between    between
Unactive      |   locked    LOCKED   between     error     locked
Under         |   error     locked   between   unlocked     error

ADC channels are 12-bit, hence ADC counts range 0..4095.  Define
"Under" through "Over" subranges:
*/

// clang-format off
// Note these values from Hessah of PSAS, cerca 2025-09-25.  Set of like data
// points from a second tested ERS board differ.

#define HALL_1_LIMIT_V_UNDER  750 // 600 . . Hall state 0
#define HALL_1_LIMIT_INACTIVE 920 // 700 . . Hall state 1
#define HALL_1_LIMIT_BETWEEN 1600 //     . . Hall state 2
#define HALL_1_LIMIT_ACTIVE  3100 //     . . Hall state 3
//                   V_OVER              . . Hall state 4

// Values observed in rocket room on 2025-10-21 TUE, (need to identify lock ring, there are three):
#define HALL_2_LIMIT_V_UNDER  800
#define HALL_2_LIMIT_INACTIVE 950
#define HALL_2_LIMIT_BETWEEN 2600
#define HALL_2_LIMIT_ACTIVE  3100

// ADC Hall sensor readings above HALL_1_LIMIT_ACTIVE considered "Over".
// Same for Hall sensor 2 readings.
// clang-format on

// TODO [ ] We have multiple code elements related to Hall sensor limits, the
//  limits of where the sensor is in proximity to a permanent magnet on the
//  locking ring.  These elements are:
//
//     (1) Hall general limits enumeration elements
//     (2) specific Hall sensor state symbols
//     (3) specific Hall sensor limit default values
//     (4) specific Hall sensor limit names
//     (5) Hall sensor state enumetation
//     (6) Hall sensor state names
//

// - STEP 2 -

// Define an outer macro which encapsulates empirically determined Hall sensor limit values:

#if 0
#define HALL_SENSORS_DEFAULT_LIMIT_VALUES \
HALL_SENSOR_INST(1, 750, 920, 1600, 3100) \
HALL_SENSOR_INST(2, 800, 950, 2600, 3100)
#endif // 0

// - STEP 1 -

// (1) limit enum,
// (2) per sensor limit enum,
// (3) per sensor limit value,
// (4) settings keyname for per sensor limit value,
// (5) Hall sensor state (Hall output)

#define HALL_SENSOR_INST(idx, lim1, lim2, lim3, lim4) \
HALL_SENSOR_CONSTRUCT(HL_V_UNDER,   HALL_##idx##_LIMIT_V_UNDER, lim1, "hall-##idx##-limit-v-under",  HALL_OUTPUT_UNDER_VOLTAGE) \
HALL_SENSOR_CONSTRUCT(HL_INACTIVE, HALL_##idx##_LIMIT_INACTIVE, lim2, "hall-##idx##-limit-inactive", HALL_OUTPUT_INACTIVE)      \
HALL_SENSOR_CONSTRUCT(HL_BETWEEN,   HALL_##idx##_LIMIT_BETWEEN, lim3, "hall-##idx##-limit-between",  HALL_OUTPUT_BETWEEN)       \
HALL_SENSOR_CONSTRUCT(HL_ACTIVE,     HALL_##idx##_LIMIT_ACTIVE, lim4, "hall-##idx##-limit-active",   HALL_OUTPUT_ACTIVE)
//                    ^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^
//                     |             |                           |      |                             |
//                    limits enum   per sensor limits enum      value  settings keyname              Hall state enum
//                    (limit ids)

#define HALL_SENSOR_CONSTRUCT(limit, per_sensor_limit, value, keyname, hall_state) limit,

enum hall_sensor_limit_ids {
HALL_SENSOR_INST(0, 0, 0, 0, 0)
HALL_SENSOR_LIMIT_COUNT
};

#undef HALL_SENSOR_CONSTRUCT

#endif // ERS_CONFIG_DEFAULTS
