/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(arbiter, CONFIG_ARBITER_LOG_LEVEL);

#include <arbiter.h>
#include <ers-can.h>
#include <ers-config-defaults.h>
#include <ers-dac.h>
#include <gpio-in.h>
#include <keeper.h>
#include "settings-ers.h"

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

// Select an ERS battery voltage "ok" threshold of 9.0 VDC, in tenths of a volt:
#define BATTERY_VOLTAGE_OK_THRESHOLD_TENTHS_V 90

#define BASE_10 10

#define ERS_ARBITER_SLEEP_PERIOD_MS 2000

#define RING_POS_PERIOD_MS 2000

#define DEV_DETERMINE_RING_POSITION_IN_MAIN_LOOP

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

K_THREAD_STACK_DEFINE(arbiter_thread_stack, CONFIG_ARBITER_THREAD_STACK_SIZE);
struct k_thread arbiter_thread_data;

static enum lock_ring_position ring_position_fs = RING_POSITION_UNKNOWN;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t determine_which_sensor(const char *sensor_name, enum hall_sensor_ids *idx)
{
	if (strncmp("s1", sensor_name, sizeof("s1")) == 0) {
		*idx = HALL_SENSOR_1;
	}
	else if (strncmp("s2", sensor_name, sizeof("s2")) == 0) {
		*idx = HALL_SENSOR_2;
	}
	else {
		return -EINVAL;
	}
	return 0;
}

// Routines to accept and store Hall sensor limits

// TODO [ ] Add check of 'endptr' to determine whether we got valid numeric input,
//  in all routines which call strtol():

int32_t cmd_set_limit_v_under(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_ids sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'v_under' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HL_V_UNDER, value);

	return 0;
}

int32_t cmd_set_limit_inactive(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_ids sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'inactive' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HL_INACTIVE, value);

	return 0;
}

int32_t cmd_set_limit_between(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_ids sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'between' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HL_BETWEEN, value);

	return 0;
}

int32_t cmd_set_limit_active(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	enum hall_sensor_ids sensor_idx;

	int32_t rc = determine_which_sensor(argv[1], &sensor_idx);
	if (rc != 0) {
		return -EINVAL;
	}

	str = argv[2];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "setting Hall sensor %d limit 'active' to %u\n",
		      (sensor_idx + 1), value);
	set_hall_sensor_limit(sensor_idx, HL_ACTIVE, value);
	return 0;
}

int32_t cmd_save_hall_limits_to_flash(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;
	int32_t rc = 0;

	// shell_fprintf(shell, SHELL_NORMAL, "- STUB -\n");

	get_hall_sensor_limit(HALL_SENSOR_1, HL_V_UNDER, &v_under_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_INACTIVE, &inactive_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_BETWEEN, &between_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_ACTIVE, &active_limit);



	// rc = store_ers_setting("",

	return rc;
}

/**
 * @brief Routine to report Hall sensor state cutoff values (in ADC counts).
 */

void arbiter_show_hall_state_limits(const struct shell *shell)
{
	uint32_t v_under_limit, inactive_limit, between_limit, active_limit;

	get_hall_sensor_limit(HALL_SENSOR_1, HL_V_UNDER, &v_under_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_INACTIVE, &inactive_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_BETWEEN, &between_limit);
	get_hall_sensor_limit(HALL_SENSOR_1, HL_ACTIVE, &active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor state limit values (in ADC "
	  "counts):\n");
	shell_fprintf(shell, SHELL_NORMAL, "  v under limit sensor 1: %u\n", v_under_limit);
	shell_fprintf(shell, SHELL_NORMAL, " inactive limit sensor 1: %u\n", inactive_limit);
	shell_fprintf(shell, SHELL_NORMAL, "  between limit sensor 1: %u\n", between_limit);
	shell_fprintf(shell, SHELL_NORMAL, "   active limit sensor 1: %u\n", active_limit);

	get_hall_sensor_limit(HALL_SENSOR_2, HL_V_UNDER, &v_under_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HL_INACTIVE, &inactive_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HL_BETWEEN, &between_limit);
	get_hall_sensor_limit(HALL_SENSOR_2, HL_ACTIVE, &active_limit);

	shell_fprintf(shell, SHELL_NORMAL, "  v under limit sensor 2: %u\n", v_under_limit);
	shell_fprintf(shell, SHELL_NORMAL, " inactive limit sensor 2: %u\n", inactive_limit);
	shell_fprintf(shell, SHELL_NORMAL, "  between limit sensor 2: %u\n", between_limit);
	shell_fprintf(shell, SHELL_NORMAL, "   active limit sensor 2: %u\n", active_limit);
}

/**
 * @brief Shell wrapper function to restore default Hall sensor limit values.
 */

void cmd_set_default_limits(const struct shell *shell, size_t argc, char **argv)
{
	LOG_INF("Setting Hall sensor limit default values . . .");
	int32_t rc = set_hall_sensor_default_limits();
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

// TODO [ ] determine whether this routine should be private:

int32_t adc_reading_to_hall_state(const enum hall_sensor_ids sensor_idx,
				  const uint32_t adc_reading,
				  enum hall_sensor_state *state)
{
	uint32_t limit_v_under, limit_inactive, limit_between, limit_active;

	if ((sensor_idx < 0) || (sensor_idx >= HALL_SENSOR_COUNT)) {
		return -EINVAL;
	}

	get_hall_sensor_limit(sensor_idx, HL_V_UNDER, &limit_v_under);
	get_hall_sensor_limit(sensor_idx, HL_INACTIVE, &limit_inactive);
	get_hall_sensor_limit(sensor_idx, HL_BETWEEN, &limit_between);
	get_hall_sensor_limit(sensor_idx, HL_ACTIVE, &limit_active);

	if (adc_reading < limit_v_under) {
		*state = HALL_OUTPUT_UNDER_VOLTAGE;
	} else if (adc_reading < limit_inactive) {
		*state = HALL_OUTPUT_INACTIVE;
	} else if (adc_reading < limit_between) {
		*state = HALL_OUTPUT_BETWEEN;
	} else if (adc_reading < limit_active) {
		*state = HALL_OUTPUT_ACTIVE;
	} else {
		*state = HALL_OUTPUT_OVER_VOLTAGE;
	}

	return 0;
}

/**
 * @brief Routine to determine lock ring position.
 *
 * @note This routine determines ring position as described in
 *  https://docs.google.com/document/d/1DnytDlZa1X-BaIqlIBrfcuedKocKrCpTfgMspk0twxI/edit?tab=t.0#heading=h.rg42p47rcyt5,
 *  and further it determines lock ring "status" as described in
 *  the same document.  Second parameter is a simplified version
 *  of the first "position" parameter.
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
	if (rc != 0) {
		LOG_ERR("determine ring position could not get hall readings, err %d", rc);
		goto done;
	}

	rc = adc_reading_to_hall_state(HALL_SENSOR_1, hall_1_reading, &hall_1_state);
	if (rc != 0) {
		LOG_ERR("Failed to get hall sensor 1 state from reading comparison, err %d", rc);
		return rc;
	}

	rc = adc_reading_to_hall_state(HALL_SENSOR_2, hall_2_reading, &hall_2_state);
	if (rc != 0) {
		LOG_ERR("Failed to get hall sensor 2 state from reading comparison, err %d", rc);
		return rc;
	}

LOG_INF("readings, states: %u %u  %d %d", hall_1_reading, hall_2_reading, hall_1_state, hall_2_state);

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
	if ((hall_1_state == HALL_OUTPUT_BETWEEN) || (hall_2_state == HALL_OUTPUT_BETWEEN)) {
		LOG_INF("H1");
		*ring_position = RING_BETWEEN_L_AND_U;
		goto qualify_validity;
	}

	// Cover error possibilities:
	if (hall_1_state == hall_2_state) {
		LOG_INF("H2");
		*ring_position = RING_POSITION_UNKNOWN;
		goto done;
	}

	// Cover row "Ina" locked positions with partial validity:
	if ((hall_1_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_2_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_2_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		LOG_INF("H3");
		*ring_position = RING_LOCKED;
		goto done;
	}

	// Cover column "Ina" unlocked positions with partial validity:
	if ((hall_2_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_1_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_1_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		LOG_INF("H4");
		*ring_position = RING_UNLOCKED;
		goto done;
	}

	// Cover row "Act" unlocked positions with partial validity:
	if ((hall_1_state == HALL_OUTPUT_INACTIVE) &&
	    ((hall_2_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_2_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		LOG_INF("H5");
		*ring_position = RING_UNLOCKED;
		goto done;
	}

	// Cover column "Act" locked positions with partial validity:
	if ((hall_2_state == HALL_OUTPUT_ACTIVE) &&
	    ((hall_1_state == HALL_OUTPUT_UNDER_VOLTAGE) ||
	     (hall_1_state == HALL_OUTPUT_OVER_VOLTAGE)))
	{
		LOG_INF("H6");
		*ring_position = RING_LOCKED;
		goto done;
	}

qualify_validity:
	if ((hall_1_state == HALL_OUTPUT_BETWEEN) && (hall_2_state == HALL_OUTPUT_BETWEEN)) {
		LOG_INF("both hall in between");
		*ring_position = RING_BETWEEN_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_OUTPUT_ACTIVE) && (hall_2_state == HALL_OUTPUT_INACTIVE)) {
		LOG_INF("ring unlocked, fully qualified");
		*ring_position = RING_UNLOCKED_FULLY_QUALIFIED;
	}

	if ((hall_1_state == HALL_OUTPUT_INACTIVE) && (hall_2_state == HALL_OUTPUT_ACTIVE)) {
		LOG_INF("ring locked, fully qualified");
		*ring_position = RING_LOCKED_FULLY_QUALIFIED;
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
	case RING_UNLOCKED:
	case RING_UNLOCKED_FULLY_QUALIFIED:
		ring_state = RING_STATE_UNLOCKED;
		break;
	case RING_BETWEEN_L_AND_U:
	case RING_BETWEEN_FULLY_QUALIFIED:
		ring_state = RING_STATE_BETWEEN;
		break;
	case RING_LOCKED:
	case RING_LOCKED_FULLY_QUALIFIED:
		ring_state = RING_STATE_LOCKED;
		break;
	case RING_POSITION_UNKNOWN:
	default:
		ring_state = RING_STATE_UNKNOWN;
	}

	ekset_ring_status(ring_state);
done:
	return rc;
}

// TODO [ ] fix ring_pos_to_str() routine, does not appear to return correct string.

char *ring_pos_to_str(const enum lock_ring_position pos)
{
        switch (pos) {
        case RING_LOCKED:
                return "ring locked";
		break;
        case RING_BETWEEN_L_AND_U:
                return "ring between";
		break;
        case RING_UNLOCKED:
                return "ring unlocked";
		break;
        case RING_LOCKED_FULLY_QUALIFIED:
                return "ring locked (fully qualified)";
		break;
        case RING_BETWEEN_FULLY_QUALIFIED:
                return "ring between (fully qualified)";
		break;
        case RING_UNLOCKED_FULLY_QUALIFIED:
                return "ring unlocked (fully qualified)";
		break;
	case RING_POSITION_UNKNOWN:
        default:
                return "ring position unknown";
        }
}

//----------------------------------------------------------------------
// - SECTION - arbiter scheduled elements
//----------------------------------------------------------------------

// Here define a routine to submit to Zephyr's work queue.

void determine_ring_pos_work_handler(struct k_work *work)
{
	static uint32_t call_count = 0;

	// call arbiter_determine_ring_state(enum lock_ring_position *ring_position)
	call_count++;
	int32_t rc = arbiter_determine_ring_state(&ring_position_fs);
	if (rc != 0) {
		LOG_ERR("Failed to figure lock ring position, error %d", rc);
	}

	set_detected_ring_position(ring_position_fs);
}

K_WORK_DEFINE(determine_ring_pos_work, determine_ring_pos_work_handler);

struct k_work_sync work_sync;

atomic_t ring_pos_work_status = ATOMIC_INIT(0);

void ring_position_timer_handler(struct k_timer *dummy)
{
	LOG_INF("M7");
	bool flush_result = k_work_flush(&determine_ring_pos_work, &work_sync);
	LOG_INF("call to k_work_flush returns %d", flush_result);

	int32_t rc = k_work_submit(&determine_ring_pos_work);
	if (rc < 0) {
		// LOG_ERR("Failed to submit to work queue, err %d", rc);
		atomic_set(&ring_pos_work_status, 1);
	} else {
		// LOG_ERR("work queue submission call returns status %d", rc);
		atomic_set(&ring_pos_work_status, 0);
	}
}

K_TIMER_DEFINE(ring_position_timer, ring_position_timer_handler, NULL);

/**
 * @brief routine to change interval for lock ring position detection at
 *   run time.
 *
 * @return 0 without condition. 
 */

int32_t update_ring_position_detection_timer(const uint32_t timeout_ms)
{
	LOG_INF("called to update ring position timer, requested interval %u ms", timeout_ms);

	k_timer_stop(&ring_position_timer);

	if (timeout_ms > 0) {
		LOG_INF("M6 - %u ms", timeout_ms);
		k_timer_start(&ring_position_timer, K_MSEC(100), K_MSEC(timeout_ms));
	} else {
		LOG_INF("Leaving timer stopped per request for zero length interval.");
	}

	return 0;
}

/**
 * @brief Routine to apply a threshold test to latest battery voltage and to
 *   update a flag to indicate whether batter voltage ok.
 */

int32_t determine_batt_ok(void)
{
	int32_t batt_voltage_in_tenths_v = 0;
	ekget_batt_read_dv(&batt_voltage_in_tenths_v);
	if (batt_voltage_in_tenths_v >= BATTERY_VOLTAGE_OK_THRESHOLD_TENTHS_V) {
		ekset_batt_ok(1);
	} else {
		ekset_batt_ok(0);
	}

	return 0;
}

int32_t calc_battery_voltage(void)
{
	uint32_t adc_reading = 0;
	float battery_voltage = 0.0;
	uint32_t battery_voltage_dv = 0;

	ekget_batt_read(&adc_reading);

	battery_voltage = (double)(((double)adc_reading / (double)4096 *3.3) / 0.2326);
	battery_voltage_dv = round(battery_voltage * 10);
	ekset_batt_read_dv(battery_voltage_dv);
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

	static uint32_t loop_count = 0;
	int32_t rc = 0;

	while (1) {
#ifdef DEV_DETERMINE_RING_POSITION_IN_MAIN_LOOP
		enum lock_ring_position ring_position = RING_POSITION_UNKNOWN;
		// char lbuf[50] = {0};
		char *str_ptr = ring_pos_to_str(ring_position);

		rc = arbiter_determine_ring_state(&ring_position);
		LOG_INF("ring state:  %s (%d)", str_ptr, ring_position);
#endif

// TODO [ ] Call battery state determination code
		rc = calc_battery_voltage();
		LOG_INF("calc battery voltage returns status %d", rc);

		rc = determine_batt_ok();

		// 1 = if battery OK and locked and has received Telemetrum Sender message within the last 2 seconds
		uint32_t battery_ok = 0;
		uint32_t can_bus_ok = 0;
		enum lock_ring_state ring_state = RING_STATE_UNKNOWN;

		ekget_batt_ok(&battery_ok);
		ekget_can_bus_ok(&can_bus_ok);
		ekget_ring_status(&ring_state);

		LOG_INF("- DEV 0105 - determining ERS ready state . . .");
		if (battery_ok && can_bus_ok && (ring_state == RING_STATE_LOCKED)) {
			ekset_ready_state(true);
		} else {
			ekset_ready_state(false);
		}

		loop_count++;
		k_msleep(ERS_ARBITER_SLEEP_PERIOD_MS);
	}
}

//----------------------------------------------------------------------
// - SECTION - init code
//----------------------------------------------------------------------

int32_t ers_init_arbiter(void)
{
	int32_t rc = 0;

	k_tid_t arbiter_tid = k_thread_create(&arbiter_thread_data, arbiter_thread_stack,
					K_THREAD_STACK_SIZEOF(arbiter_thread_stack),
					arbiter_thread_entry, NULL, NULL, NULL,
					CONFIG_ARBITER_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!arbiter_tid) {
		LOG_ERR("ERROR spawning arbiter thread\n");
	}

#ifndef DEV_DETERMINE_RING_POSITION_IN_MAIN_LOOP
	k_timer_start(&ring_position_timer, K_MSEC(RING_POS_PERIOD_MS), K_MSEC(RING_POS_PERIOD_MS));
#endif

	return rc;
}
