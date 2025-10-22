/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(arbiter, LOG_LEVEL_INF);

#include <ers-can.h>
#include <ers-dac.h>
#include <gpio-in.h>
#include <keeper.h>
#include <arbiter.h>

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

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
#define HALL_READING_V_UNDER_CUTOFF   600
#define HALL_READING_INACTIVE_CUTOFF  700
#define HALL_READING_BETWEEN_CUTOFF  1600
#define HALL_READING_ACTIVE_CUTOFF   3100
// ADC Hall sensor readings above HALL_READING_ACTIVE_CUTOFF considered "Over".
// clang-format on

#define BASE_10 10

#define ERS_ARBITER_SLEEP_PER_MS 2000

#define RING_POS_PERIOD_MS 10

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define ARBITER_THREAD_STACK_SIZE 512
#define ARBITER_THREAD_PRIORITY 1
K_THREAD_STACK_DEFINE(arbiter_thread_stack, ARBITER_THREAD_STACK_SIZE);
struct k_thread arbiter_thread_data;

static atomic_t hall_reading_v_under_cutoff_fs = ATOMIC_INIT(HALL_READING_V_UNDER_CUTOFF);
static atomic_t hall_reading_inactive_cutoff_fs = ATOMIC_INIT(HALL_READING_INACTIVE_CUTOFF);
static atomic_t hall_reading_between_cutoff_fs = ATOMIC_INIT(HALL_READING_BETWEEN_CUTOFF);
static atomic_t hall_reading_active_cutoff_fs = ATOMIC_INIT(HALL_READING_ACTIVE_CUTOFF);

static enum lock_ring_position ring_position_fs = RING_POSITION_UNKNOWN;

void arbiter_set_hall_state_cutoff_defaults(void);

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// Routines to accept and store Hall sensor threasholds, such as under voltage.

// - SECTION - setter functions for hall state cutuffs

void arbiter_set_v_under_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_v_under_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_inactive_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_inactive_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_between_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_between_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_active_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_active_cutoff_fs, (atomic_val_t)value);
}

// TODO [ ] Add check of 'endptr' to determine whether we got valid numeric input,
//  in all routines which call strtol():

void sw_set_v_under_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "from user got v_under_cutoff of %u\n",
		      value);
	arbiter_set_v_under_cutoff(value);
}

void sw_set_inactive_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_inactive_cutoff(value);
}

void sw_set_between_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_between_cutoff(value);
}

void sw_set_active_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_active_cutoff(value);
}

// TODO [ ] make following routine name match local naming convention 

void arbiter_set_hall_state_cutoff_defaults(void)
{
	arbiter_set_v_under_cutoff(HALL_READING_V_UNDER_CUTOFF);
	arbiter_set_inactive_cutoff(HALL_READING_INACTIVE_CUTOFF);
	arbiter_set_between_cutoff(HALL_READING_BETWEEN_CUTOFF);
	arbiter_set_active_cutoff(HALL_READING_ACTIVE_CUTOFF);
}

void sw_set_default_cutoffs(const struct shell *shell, size_t argc, char **argv)
{
	LOG_INF("Setting Hall cutoffs to default values . . .");
	arbiter_set_hall_state_cutoff_defaults();
	arbiter_show_hall_state_cutoffs(shell);
}

// - SECTION - getter functions for hall state cutuffs:

void arbiter_get_v_under_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_v_under_cutoff_fs);
}

void arbiter_get_inactive_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_inactive_cutoff_fs);
}

void arbiter_get_between_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_between_cutoff_fs);
}

void arbiter_get_active_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_active_cutoff_fs);
}

/**
 * @brief Routine to report Hall sensor state cutoff values (in ADC counts).
 */

void arbiter_show_hall_state_cutoffs(const struct shell *shell)
{
	uint32_t v_under_cutoff, inactive_cutoff, between_cutoff, active_cutoff;

	arbiter_get_v_under_cutoff(&v_under_cutoff);
	arbiter_get_inactive_cutoff(&inactive_cutoff);
	arbiter_get_between_cutoff(&between_cutoff);
	arbiter_get_active_cutoff(&active_cutoff);

	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor state cutoff values (in ADC "
	  "counts):\n");
	shell_fprintf(shell, SHELL_NORMAL, "  v under cutoff: %u\n", v_under_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, " inactive cutoff: %u\n", inactive_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, "  between cutoff: %u\n", between_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, "   active cutoff: %u\n", active_cutoff);
}

/**
 * @brief Routine to categorize ADC readings of lock ring Hall sensor into one
 *   of five states.
 */

enum hall_sensor_state adc_reading_to_hall_state(const uint32_t adc_reading)
{
	enum hall_sensor_state sensor_state = HALL_OUTPUT_OVER_VOLTAGE;

	if (adc_reading < HALL_READING_V_UNDER_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_UNDER_VOLTAGE;
	}

	if (adc_reading < HALL_READING_INACTIVE_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_INACTIVE;
	}

	if (adc_reading < HALL_READING_BETWEEN_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_BETWEEN;
	}

	if (adc_reading < HALL_READING_ACTIVE_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_ACTIVE;
	}

	return sensor_state;
}

/**
 * @brief Routine to determine lock ring position.
 *
 * @note Calling code is responsible for setting parameter ring_position to
 *    a sensible starting value, namely 'RING_POSITION_UNKNOWN'.
 */

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position)
{
	int32_t rc = 0;
	uint32_t hall_1_reading = 0;
	uint32_t hall_2_reading = 0;
	enum hall_sensor_state hall_1_state = HALL_OUTPUT_UNKNOWN;
	enum hall_sensor_state hall_2_state = HALL_OUTPUT_UNKNOWN;

	rc = ekget_both_hall_sensors(&hall_1_reading, &hall_2_reading);
	if (rc != 0)
	{
		LOG_ERR("determine ring position could not get hall readings, error %d", rc);
		goto done;
	}

	hall_1_state = adc_reading_to_hall_state(hall_1_reading);
	hall_2_state = adc_reading_to_hall_state(hall_2_reading);

/*
   Hall2   Vun   Ina   Bet   Act   Ovr 
Hall1     ----- ----- ----- ----- -----
-----
 Vun        x     U     B     L     x   
 Ina        L     x     B     Lf    L   
 Bet        B     B     Bf    B     B   
 Act        U     Uf    B     x     U   
 Ovr        x     U     B     L     x

Note Uf, Bf, Lf are fully qualified unlocked, in between and lock ring position
determinations.
*/

	// Look for possible "between" sensor values pairs first:
	if ((hall_1_state == HALL_OUTPUT_BETWEEN) || (hall_2_state == HALL_OUTPUT_BETWEEN))
	{
		*ring_position = RING_BETWEEN_L_AND_U;
		goto qualify_validity;
	}

	// Cover error possibilities:
	if (hall_1_state == hall_2_state)
	{
		*ring_position = RING_POSITION_UNKNOWN;
		goto done;
	}

	// Cover row "Ina" locked positions with partial validity:
	if ((hall_1_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_2_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_2_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		*ring_position = RING_LOCKED;
		goto done;
	}

	// Cover column "Ina" unlocked positions with partial validity:
	if ((hall_2_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_1_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_1_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		*ring_position = RING_UNLOCKED;
		goto done;
	}

	// Cover row "Act" unlocked positions with partial validity:
	if ((hall_1_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_2_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_2_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		*ring_position = RING_UNLOCKED;
		goto done;
	}

	// Cover column "Act" locked positions with partial validity:
	if ((hall_2_state == HALL_OUTPUT_ACTIVE) &&
	    ((hall_1_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_1_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		*ring_position = RING_LOCKED;
		goto done;
	}

qualify_validity:
	if ((hall_1_state == HALL_OUTPUT_BETWEEN) && (hall_2_state == HALL_OUTPUT_BETWEEN))
	{
		*ring_position = RING_BETWEEN_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_OUTPUT_ACTIVE) && (hall_2_state == HALL_OUTPUT_INACTIVE))
	{
		*ring_position = RING_UNLOCKED_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_OUTPUT_INACTIVE) && (hall_2_state == HALL_OUTPUT_ACTIVE))
	{
		*ring_position = RING_LOCKED_FULLY_QUALIFIED;
	}

done:
	return rc;
}

char *ring_pos_to_str(enum lock_ring_position pos)
{
        switch (pos) {
        case RING_LOCKED:
                return "ring locked";
        case RING_BETWEEN_L_AND_U:
                return "ring between";
        case RING_UNLOCKED:
                return "ring unlocked";
        case RING_LOCKED_FULLY_QUALIFIED:
                return "ring locked (fully qualified)";
        case RING_BETWEEN_FULLY_QUALIFIED:
                return "ring between (fully qualified)";
        case RING_UNLOCKED_FULLY_QUALIFIED:
                return "ring unlocked (fully qualified)";
	case RING_POSITION_UNKNOWN:
        default:
                return "ring position unknown";
        }
}

//----------------------------------------------------------------------
// - SECTION - arbiter scheduled elements
//----------------------------------------------------------------------

// Here define a routine to submit to Zephyr's work queue, followed
// by a kernel time which calls the API to submit that work:

void determine_ring_pos_work_handler(struct k_work *work)
{
	static uint32_t call_count = 0;

	// call arbiter_determine_ring_state(enum lock_ring_position *ring_position)
	call_count++;
	int32_t rc = arbiter_determine_ring_state(&ring_position_fs);
	if (rc != 0)
	{
		LOG_ERR("Failed to figure lock ring position, error %d", rc);
	}
	// TODO [ ] remove following LOG_INF() call in production code:
	// TODO [ ] convert ring_position_fs to atomic type.
#if 1
	else
	{
		// LOG_INF("Lock ring position is %d", ring_position_fs);
		if ((call_count % 200) == 0)
		{
			LOG_INF("arbiter determine ring position called %u times", call_count);
		}
	}
#endif // 0
}

K_WORK_DEFINE(determine_ring_pos_work, determine_ring_pos_work_handler);

void ring_position_timer_handler(struct k_timer *dummy)
{
        k_work_submit(&determine_ring_pos_work);
}

K_TIMER_DEFINE(ring_position_timer, ring_position_timer_handler, NULL);

void arbiter_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	static uint32_t loop_count = 0;
	enum lock_ring_position ring_position = RING_POSITION_UNKNOWN;
	int32_t rc = 0;

	while (1)
	{
		// LOG_INF("M3");
#if 0
		LOG_INF("setting deploy1 GPIO to %d", (loop_count % 2));
		rc = ers_gpios_set_deploy1(loop_count % 2);
		LOG_INF("GPIO set returns status %d", rc);
		rc = ers_gpios_set_deploy2((loop_count + 1) % 2);
		LOG_INF("GPIO set returns status %d", rc);
#endif

// TODO [x] Call ring state determination code
		rc = arbiter_determine_ring_state(&ring_position);
		// LOG_INF("Current lock ring position:  %d", ring_position);

// TODO [ ] Call battery state determination code

		loop_count++;
		k_msleep(ERS_ARBITER_SLEEP_PER_MS);
	}
}

//----------------------------------------------------------------------
// - SECTION - init code
//----------------------------------------------------------------------

int32_t ers_init_arbiter(void)
{
	int32_t rc = 0;

	// Initialize Hall sensor ADC count threshold values:
	// (These values used to determine practical Hall sensor states)
	arbiter_set_hall_state_cutoff_defaults();

	k_tid_t arbiter_tid = k_thread_create(&arbiter_thread_data, arbiter_thread_stack,
					K_THREAD_STACK_SIZEOF(arbiter_thread_stack),
					arbiter_thread_entry, NULL, NULL, NULL,
				 	ARBITER_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!arbiter_tid) {
		LOG_ERR("ERROR spawning arbiter thread\n");
	}

	k_timer_start(&ring_position_timer, K_MSEC(RING_POS_PERIOD_MS), K_MSEC(RING_POS_PERIOD_MS));

	return rc;
}
