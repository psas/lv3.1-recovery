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

#define HALL_1_LIMIT_V_UNDER  600
#define HALL_1_LIMIT_INACTIVE 700
#define HALL_1_LIMIT_BETWEEN 1600
#define HALL_1_LIMIT_ACTIVE  3100

// Values observed in rocket room on 2025-10-21 TUE, (need to identify lock ring, there are three):
#define HALL_2_LIMIT_V_UNDER  800
#define HALL_2_LIMIT_INACTIVE 875
#define HALL_2_LIMIT_BETWEEN 2350
#define HALL_2_LIMIT_ACTIVE  2550

// ADC Hall sensor readings above HALL_1_LIMIT_ACTIVE considered "Over".
// Same for Hall sensor 2 readings.
// clang-format on

#endif // ERS_CONFIG_DEFAULTS
