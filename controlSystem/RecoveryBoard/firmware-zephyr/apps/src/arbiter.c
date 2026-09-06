/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "arbiter.h"
#include "can-ers.h"
#include "dac-ers.h"
#include "gpio-in.h"
#include "hall-and-ring.h"
#include "keeper.h"
#include "settings-ers.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// LOG_MODULE_REGISTER(arbiter, CONFIG_ARBITER_LOG_LEVEL);
LOG_MODULE_REGISTER(arbiter, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

// Select an ERS battery voltage "ok" threshold of 9.0 VDC, in tenths of a volt:
#define BATTERY_VOLTAGE_OK_THRESHOLD_TENTHS_V 90

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

K_THREAD_STACK_DEFINE(arbiter_thread_stack, CONFIG_ARBITER_THREAD_STACK_SIZE);
struct k_thread arbiter_thread_data;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t determine_which_sensor(const char *sensor_name, enum hall_sensor_instances *sensor_idx)
{
	if (strncmp("s1", sensor_name, sizeof("s1")) == 0) {
		*sensor_idx = HALL_SENSOR_1;
	}
	else if (strncmp("s2", sensor_name, sizeof("s2")) == 0) {
		*sensor_idx = HALL_SENSOR_2;
	}
	else {
		return -EINVAL;
	}
	return 0;
}

/**
 * @brief Routine to report Hall sensor state cutoff values (in ADC counts).
 */

void arbiter_show_hall_state_limits(const struct shell *shell)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;

	keeper_get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_UNDER, &v_under_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_1, HALL_STATE_V_INACTIVE, &inactive_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_BETWEEN, &between_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_1, HALL_LIMIT_V_ACTIVE, &active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor state limit values (in ADC "
	  "counts):\n");
	shell_fprintf(shell, SHELL_NORMAL, "  v under limit sensor 1: %u\n", v_under_limit);
	shell_fprintf(shell, SHELL_NORMAL, " inactive limit sensor 1: %u\n", inactive_limit);
	shell_fprintf(shell, SHELL_NORMAL, "  between limit sensor 1: %u\n", between_limit);
	shell_fprintf(shell, SHELL_NORMAL, "   active limit sensor 1: %u\n", active_limit);

	keeper_get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_UNDER, &v_under_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_2, HALL_STATE_V_INACTIVE, &inactive_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_BETWEEN, &between_limit);
	keeper_get_hall_sensor_limit(HALL_SENSOR_2, HALL_LIMIT_V_ACTIVE, &active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "  v under limit sensor 2: %u\n", v_under_limit);
	shell_fprintf(shell, SHELL_NORMAL, " inactive limit sensor 2: %u\n", inactive_limit);
	shell_fprintf(shell, SHELL_NORMAL, "  between limit sensor 2: %u\n", between_limit);
	shell_fprintf(shell, SHELL_NORMAL, "   active limit sensor 2: %u\n", active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "- DEV 0212 - keyname '%s'\n", SETTING_KEYNAME_S2_HL1);
}

/**
 * @brief Shell wrapper function to restore default Hall sensor limit values.
 */

void arbiter_cmd_set_default_limits(const struct shell *shell, size_t argc, char **argv)
{
	shell_print(shell, "Setting Hall sensor limit default values . . .");
	int32_t rc = keeper_set_hall_sensor_default_limits();
	if (rc != 0) {
		LOG_ERR("Failed to set hall limit default values, err %d", rc);
	} else {
		arbiter_show_hall_state_limits(shell);
	}
}

/**
 * @brief Routine to categorize ADC readings of lock ring Hall sensor into one
 *   of five states.
 */

static int32_t adc_reading_to_hall_state(const enum hall_sensor_instances sensor_idx,
				  const uint32_t adc_reading,
				  enum hall_sensor_state_ids *state)
{
	uint32_t limit_v_under, limit_inactive, limit_between, limit_active;

	if ((sensor_idx < 0) || (sensor_idx >= HALL_SENSOR_COUNT)) {
		return -EINVAL;
	}

	keeper_get_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_UNDER, &limit_v_under);
	keeper_get_hall_sensor_limit(sensor_idx, HALL_STATE_V_INACTIVE, &limit_inactive);
	keeper_get_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_BETWEEN, &limit_between);
	keeper_get_hall_sensor_limit(sensor_idx, HALL_LIMIT_V_ACTIVE, &limit_active);

	if (adc_reading < limit_v_under) {
		*state = HALL_STATE_V_UNDER;
	} else if (adc_reading < limit_inactive) {
		*state = HALL_STATE_V_INACTIVE;
	} else if (adc_reading < limit_between) {
		*state = HALL_STATE_V_BETWEEN;
	} else if (adc_reading < limit_active) {
		*state = HALL_STATE_V_ACTIVE;
	} else {
		*state = HALL_STATE_V_OVER;
	}

	return 0;
}

#define DEV_ARB_MESG_SIZE 256

/**
 * @brief Routine to print arbiter messages to logging backend, with a run time
 *   check of app logging state, as held by ERS Zephyr keeper module.
 *
 * @note 'arb_mesg' is short for "arbiter message".
 */

static void arb_mesg(char *fmt, ...)
{
	int32_t rc = 0;
	static char lbuf[DEV_ARB_MESG_SIZE] = {0};
	memset(lbuf, 0, sizeof(lbuf));

	int n = 0;
	size_t size = 0;
	char *p = lbuf;
	va_list ap;

	/* Determine required size.  */

	va_start(ap, fmt);
	n = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	if (n < 0) {
		LOG_ERR("Failed to format diag message, vsnpirntf() returns %d", n);
		return;
	}

	size = (size_t) n + 1;      /* One extra byte for '\0' */
	if (size > DEV_ARB_MESG_SIZE) {
		LOG_WRN("Message truncated, is %d chars, only able to show %d",
			size, DEV_ARB_MESG_SIZE);
		size = DEV_ARB_MESG_SIZE;
	}

	va_start(ap, fmt);
	n = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	keeper_get_diag_mode(&rc);
	if (rc == true) {
		LOG_INF("(s %d) %s", size, p);
	}
}

// TODO [ ] Determine whether this API needs mutex protection, as it runs
//          longer than a few clock cycles.

int32_t arbiter_determine_ring_state(enum lock_ring_position *ring_position)
{
	int32_t rc = 0;
	uint32_t hall_1_reading = 0;
	uint32_t hall_2_reading = 0;
	enum hall_sensor_state_ids hall_1_state = HALL_STATE_UNKNOWN;
	enum hall_sensor_state_ids hall_2_state = HALL_STATE_UNKNOWN;

	rc = keeper_get_both_hall_sensors(&hall_1_reading, &hall_2_reading);
	if (rc != 0) {
		LOG_ERR("determine ring position could not get hall readings, err %d", rc);
		goto done;
	}

	rc = adc_reading_to_hall_state(HALL_SENSOR_1, hall_1_reading, &hall_1_state);
	if (rc != 0) {
		LOG_ERR("Failed to get hall sensor 1 state from reading comparison, err %d", rc);
		goto done;
	}

	rc = adc_reading_to_hall_state(HALL_SENSOR_2, hall_2_reading, &hall_2_state);
	if (rc != 0) {
		LOG_ERR("Failed to get hall sensor 2 state from reading comparison, err %d", rc);
		goto done;
	}

	arb_mesg("readings, states: %u %u  %d %d", hall_1_reading, hall_2_reading, hall_1_state, hall_2_state);

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
	if ((hall_1_state == HALL_STATE_V_BETWEEN) || (hall_2_state == HALL_STATE_V_BETWEEN)) {
		arb_mesg("H1");
		*ring_position = RING_POS_BETWEEN_L_AND_U;
		goto qualify_validity;
	}

	// Cover error possibilities:
	if (hall_1_state == hall_2_state) {
		arb_mesg("H2");
		*ring_position = RING_POS_UNKNOWN;
		goto done;
	}

	// Cover row "Ina" locked positions with partial validity:
	if ((hall_1_state == HALL_STATE_V_INACTIVE) &&
	    ((hall_2_state == HALL_STATE_V_UNDER) ||
	     (hall_2_state == HALL_STATE_V_OVER)))
	{
		arb_mesg("H3");
		*ring_position = RING_POS_LOCKED;
		goto done;
	}

	// Cover column "Ina" unlocked positions with partial validity:
	if ((hall_2_state == HALL_STATE_V_INACTIVE) &&
	    ((hall_1_state == HALL_STATE_V_UNDER) ||
	     (hall_1_state == HALL_STATE_V_OVER)))
	{
		arb_mesg("H4");
		*ring_position = RING_POS_UNLOCKED;
		goto done;
	}

	// Cover row "Act" unlocked positions with partial validity:
	if ((hall_1_state == HALL_STATE_V_INACTIVE) &&
	    ((hall_2_state == HALL_STATE_V_UNDER) ||
	     (hall_2_state == HALL_STATE_V_OVER)))
	{
		arb_mesg("H5");
		*ring_position = RING_POS_UNLOCKED;
		goto done;
	}

	// Cover column "Act" locked positions with partial validity:
	if ((hall_2_state == HALL_STATE_V_ACTIVE) &&
	    ((hall_1_state == HALL_STATE_V_UNDER) ||
	     (hall_1_state == HALL_STATE_V_OVER)))
	{
		arb_mesg("H6");
		*ring_position = RING_POS_LOCKED;
		goto done;
	}

qualify_validity:
	if ((hall_1_state == HALL_STATE_V_BETWEEN) && (hall_2_state == HALL_STATE_V_BETWEEN)) {
		arb_mesg("both hall in between");
		*ring_position = RING_POS_BETWEEN_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_STATE_V_ACTIVE) && (hall_2_state == HALL_STATE_V_INACTIVE)) {
		arb_mesg("ring unlocked, fully qualified");
		*ring_position = RING_POS_UNLOCKED_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_STATE_V_INACTIVE) && (hall_2_state == HALL_STATE_V_ACTIVE)) {
		arb_mesg("ring locked, fully qualified");
		*ring_position = RING_POS_LOCKED_FULLY_QUALIFIED;
	}

	/**
	 * @note Seems a bit duplicative but ring position from Hall sensor pair
	 *   readings is more finely defined than ring state as it appears in
	 *   CAN heartbeat-plus-update message.  Provide for the simpler ring
	 *   state reporting here in following switch construct.
	 */

	enum lock_ring_state ring_state;

	switch (*ring_position)
	{
	case RING_POS_UNLOCKED:
	case RING_POS_UNLOCKED_FULLY_QUALIFIED:
		ring_state = RING_STATE_UNLOCKED;
		break;
	case RING_POS_BETWEEN_L_AND_U:
	case RING_POS_BETWEEN_FULLY_QUALIFIED:
		ring_state = RING_STATE_BETWEEN;
		break;
	case RING_POS_LOCKED:
	case RING_POS_LOCKED_FULLY_QUALIFIED:
		ring_state = RING_STATE_LOCKED;
		break;
	case RING_POS_UNKNOWN:
	default:
		ring_state = RING_STATE_UNKNOWN;
	}

	keeper_set_ring_status(ring_state);
done:
	return rc;
}

char *arbiter_ring_pos_to_str(const enum lock_ring_position pos)
{
        switch (pos) {
        case RING_POS_LOCKED:
                return "ring locked";
		break;
        case RING_POS_BETWEEN_L_AND_U:
                return "ring between";
		break;
        case RING_POS_UNLOCKED:
                return "ring unlocked";
		break;
        case RING_POS_LOCKED_FULLY_QUALIFIED:
                return "ring locked (fully qualified)";
		break;
        case RING_POS_BETWEEN_FULLY_QUALIFIED:
                return "ring between (fully qualified)";
		break;
        case RING_POS_UNLOCKED_FULLY_QUALIFIED:
                return "ring unlocked (fully qualified)";
		break;
	case RING_POS_UNKNOWN:
        default:
                return "ring position unknown";
        }
}

/**
 * @brief Routine to apply a threshold test to latest battery voltage and to
 *   update a flag to indicate whether batter voltage ok.
 */

static int32_t determine_batt_ok(void)
{
	int32_t batt_voltage_in_tenths_v = 0;
	keeper_get_battery_decivolts(&batt_voltage_in_tenths_v);
	if (batt_voltage_in_tenths_v >= BATTERY_VOLTAGE_OK_THRESHOLD_TENTHS_V) {
		keeper_set_batt_ok(1);
	} else {
		keeper_set_batt_ok(0);
	}

	return 0;
}

int32_t calc_battery_voltage(void)
{
	uint32_t adc_reading = 0;
	float battery_voltage = 0.0;
	uint32_t battery_voltage_dv = 0;

	keeper_get_batt_read(&adc_reading);

	battery_voltage = (double)(((double)adc_reading / (double)4096 *3.3) / 0.2326);
	battery_voltage_dv = round(battery_voltage * 10);
	keeper_set_battery_decivolts(battery_voltage_dv);
	return 0;
}

//----------------------------------------------------------------------
// - SECTION - arbiter thread entry point
//----------------------------------------------------------------------

void arbiter_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	enum lock_ring_position ring_position = RING_POS_UNKNOWN;
	uint32_t battery_ok = 0;
	uint32_t can_bus_ok = 0;
	enum lock_ring_state ring_state = RING_STATE_UNKNOWN;

	static uint32_t states_check_period_ms = CONFIG_ARBITER_LOOP_SLEEP_PER_MS;
	static uint32_t loop_count = 0;
	int32_t rc = 0;

	// Note this while loop determines three important values in sequence.
	// These recovery system values are:
	//
	// - lock ring state
	// - system battery voltage
	// - "battery ok" state
	//
	// A single loop construct with a run-time adjustable period determines
	// how often these values are determined.
	//
	// While this may be good enough for timely parachute deployment, a
	// review of this design is in order.  Questions to answer include:
	//
	// - Do any of these values need a dedicated check interval?
	// - Would kernel timers be a better or good way to provide independent
	//   timing for each recovery system value?

	while (1) {
		rc = arbiter_determine_ring_state(&ring_position);
		if (rc < 0) {
			LOG_ERR("Failed to determine lock ring position, err %d", rc);
		}

		rc = calc_battery_voltage();
		if (rc < 0) {
			LOG_ERR("Failed to calculate battery voltage, err %d", rc);
		}

		rc = determine_batt_ok();
		if (rc < 0) {
			LOG_ERR("Failed to determine if battery ok, err %d", rc);
		}

		keeper_get_batt_ok(&battery_ok);
		keeper_get_can_bus_ok(&can_bus_ok);
		keeper_get_ring_status(&ring_state);

		if (battery_ok && can_bus_ok && (ring_state == RING_STATE_LOCKED)) {
			keeper_set_ready_state(true);
		} else {
			keeper_set_ready_state(false);
		}

                keeper_get_ring_pos_detection_interval(&states_check_period_ms);
		LOG_INF("A1");
		loop_count++;

		// k_msleep(CONFIG_ARBITER_LOOP_SLEEP_PER_MS);
		k_msleep(states_check_period_ms);
	}
}

//----------------------------------------------------------------------
// - SECTION - init code
//----------------------------------------------------------------------

int32_t arbiter_init(void)
{
	int32_t rc = 0;

	k_tid_t arbiter_tid = k_thread_create(&arbiter_thread_data, arbiter_thread_stack,
					K_THREAD_STACK_SIZEOF(arbiter_thread_stack),
					arbiter_thread_entry, NULL, NULL, NULL,
					CONFIG_ARBITER_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!arbiter_tid) {
		LOG_ERR("ERROR spawning arbiter thread\n");
	}

	return rc;
}
