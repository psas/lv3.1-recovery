/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief ERS drogue and main app uses Zephyr shell facility and adds custom
 *   commands to it.
 *
 * @note The code in this module is a mix of Zephyr "command set up" macros
 *   and wrapper functions.  Those wrappers often call more detailed functions
 *   in their respective ERS application modules.
 *
 * @note Zephyr shell commands execute in whichever thread or workqueue Zephyr
 *   RTOS is configured to run its shell facility.
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(shell_support, LOG_LEVEL_INF);

#include <arbiter.h>
#include <ers-adc.h>
#include <ers-dac.h>
#include <ers-util.h>
#include <keeper.h>
#include <motor-control.h>

#define SHELL_SUPPORT_THREAD_STACK_SIZE 512
#define SHELL_SUPPORT_THREAD_PRIORITY 5

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

static int cmd_wrapper_read_adc_in0(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	return cmd_ers_read_adc_in0(shell);
}

static int cmd_wrapper_read_adc_in1(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	return cmd_ers_read_adc_in1(shell);
}

//----------------------------------------------------------------------
// - SECTION - ERS diagnotics
//----------------------------------------------------------------------

static int cmd_diag_periodic_on(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	ek_sys_diag_periodic();
	return 0;
}

static int cmd_diag_periodic_off(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	ek_sys_diag_quiet();
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	ers_cmds_diag,
	SHELL_CMD_ARG(on, NULL,
		"enable ERS periodic diagnostics",
		cmd_diag_periodic_on, 0, 0),
	SHELL_CMD_ARG(off, NULL,
		"disable ERS periodic diagnostics",
		cmd_diag_periodic_off, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(diag, &ers_cmds_diag, "- ERS - diagnostics", NULL);

//----------------------------------------------------------------------
// - SECTION - ERS ADC commands
//----------------------------------------------------------------------

static int cmd_wrapper_read_adc_all(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	int32_t rc = 0;

        shell_print(shell, "Calling ADC module to read all ADC channels . . .\n");
	rc = adc_read_channels(ADC_READING_BATT_READ, ADC_READING_HALL_2);
	if (rc != 0)
	{
		shell_fprintf(shell, SHELL_NORMAL, "ADC read channels returns error status %d\n",
				 rc);
		shell_fprintf(shell, SHELL_NORMAL, "Last known good stored readings are:\n");
	}

	uint32_t a, b, c, d;
	ekget_batt_read(&a);
	ekget_motor_isense(&b);
	ekget_hall_1(&c);
	ekget_hall_2(&d);
	shell_fprintf(shell, SHELL_NORMAL, "ADC counts for batter, motor current, Hall 1, Hall 2:\n");
	shell_fprintf(shell, SHELL_NORMAL, "%u  %u  %u  %u", a, b, c, d);

	return rc;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	ers_cmds,
	SHELL_CMD_ARG(adcall, NULL,
		"Read ERS board's four ADC channels",
		cmd_wrapper_read_adc_all, 0, 0),
	SHELL_CMD_ARG(adc0, NULL,
		"Read ERS board ADC for Hall sensor 1",
		cmd_wrapper_read_adc_in0, 0, 0),
	SHELL_CMD_ARG(adc1, NULL,
		"Read ERS board ADC for Hall sensor 2",
		cmd_wrapper_read_adc_in1, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(ers, &ers_cmds, "- ERS - development commands", NULL);

//----------------------------------------------------------------------
// - SECTION - ERS Hall sensor commands
//----------------------------------------------------------------------

/**
 * @note Hall sensor commands referenced in this section are implemented in a
 *   separate ERS source file.
 */

SHELL_SUBCMD_SET_CREATE(sub_section_hall, (hall));

/* Create a set of one subcommands for 'hall' command */
SHELL_SUBCMD_SET_CREATE(sub_section_hall_set, (hall, set));

SHELL_SUBCMD_ADD((hall), show_limits, &sub_section_hall,
  "show Hall sensor limit values (ADC counts 0..4095)", arbiter_show_hall_state_limits, 1, 0);

SHELL_SUBCMD_ADD((hall), v_under_limit, &sub_section_hall_set,
  "set Hall limit 'voltage under':  hall v_under [s1|s2] [value]", cmd_set_limit_v_under, 3, 0);

SHELL_SUBCMD_ADD((hall), inactive_limit, &sub_section_hall_set,
  "set Hall state inactive limit:  hall inactive [s1|s2] [value]", cmd_set_limit_inactive, 3, 0);

SHELL_SUBCMD_ADD((hall), between_limit, &sub_section_hall_set,
  "set Hall state between limit:  hall between [s1|s2] [value]", cmd_set_limit_between, 3, 0);

SHELL_SUBCMD_ADD((hall), active_limit, &sub_section_hall_set,
  "set Hall state active limit:  hall active [s1|s2] [value]", cmd_set_limit_active, 3, 0);

SHELL_SUBCMD_ADD((hall), defaults, &sub_section_hall, "restore Hall sensor limit defaults",
  cmd_set_default_limits, 1, 0);

SHELL_CMD_REGISTER(hall, &sub_section_hall,
  "- ERS - show and set Hall sensor limit values (in ADC counts)", NULL);

//----------------------------------------------------------------------
// - SECTION - ERS lock ring commands (IN PROGRESS)
//----------------------------------------------------------------------

// TODO [ ] use the shell parameter to print queried data, to avoid the echo
//           or doubling effect when the shell "sees" log messages from Zephyr's
//          logging system.

static int cmd_show_locking_ring_pos(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	enum lock_ring_position ring_position = RING_POSITION_UNKNOWN;
	int32_t rc = arbiter_determine_ring_state(&ring_position);
	if (rc == 0)
	{
		char lbuf[SIZE_SHORT_ERS_MESSAGE] = {0};
		char *ring_pos_as_str = lbuf;
		ring_pos_as_str = ring_pos_to_str(ring_position);
		shell_fprintf(shell, SHELL_NORMAL, "Current lock ring position:  %d %s\n",
				ring_position, ring_pos_as_str);
	}
	else
	{
		shell_fprintf(shell, SHELL_NORMAL, "Failed lock ring position query, error %d\n",
				 rc);
	}

	return rc;
}

static int cmd_set_pos_detection_interval(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

        uint32_t value = 0;
        char *endptr, *str;
        int32_t rc = 0;

        str = argv[1];
        value = strtol(str, &endptr, BASE_TEN);

	shell_fprintf(shell, SHELL_NORMAL, "Storing ring position detection "
			"interval of %u ms . . .\n", value);
	set_ring_pos_detection_interval(value);
	rc = update_ring_position_detection_timer(value);
	return rc;
}

static int cmd_show_pos_detection_interval(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

        uint32_t value = 0;
	get_ring_pos_detection_interval(&value);
	shell_fprintf(shell, SHELL_NORMAL, "ring position detection interval is %u ms\n", value);
	return 0;
}


static int cmd_lock_ring(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	int32_t rc = mc_lock_ring();
	if (rc != 0)
	{
		shell_fprintf(shell, SHELL_NORMAL, "Failed to lock ring, err %d\n", rc);
	}

	return 0;
}

static int cmd_unlock_ring(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	int32_t rc = mc_unlock_ring();
	if (rc != 0)
	{
		shell_fprintf(shell, SHELL_NORMAL, "Failed to lock ring, err %d\n", rc);
	}

	return 0;
}

// clang-format off
SHELL_SUBCMD_SET_CREATE(sub_section_ring, (ring));

SHELL_SUBCMD_ADD((ring), position, &sub_section_ring, "show locking ring position",
  cmd_show_locking_ring_pos, 1, 0);

SHELL_SUBCMD_ADD((ring), dishow, &sub_section_ring, "show ring position detection internal in ms",
  cmd_show_pos_detection_interval, 1, 0);

SHELL_SUBCMD_ADD((ring), diset, &sub_section_ring, "set ring position detection internal in ms",
  cmd_set_pos_detection_interval, 2, 0);

SHELL_SUBCMD_ADD((ring), lock, &sub_section_ring, "lock ring", cmd_lock_ring, 1, 0);

SHELL_SUBCMD_ADD((ring), unlock, &sub_section_ring, "unlock ring", cmd_unlock_ring, 1, 0);

SHELL_CMD_REGISTER(ring, &sub_section_ring, "- ERS - lock ring commands", NULL);
// clang-format on

//----------------------------------------------------------------------
// - SECTION - DAC commands
//----------------------------------------------------------------------

static int cmd_dac_show_range(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	uint32_t bound_low = 0;
	uint32_t bound_high = 0;
	int32_t rc = dac_range(&bound_low, &bound_high);
	if (rc != 0)
	{
		shell_fprintf(shell, SHELL_NORMAL, "Failed to read DAC range values, err %d\n", rc);
	}
	else
	{
		shell_fprintf(shell, SHELL_NORMAL, "DAC range is %u..%u\n", bound_low, bound_high);
	}
	return 0;
}

static int cmd_dac_show_dac_setting(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	uint32_t dac_setting = 0;
	int32_t rc = dac_present_value(&dac_setting);
	if (rc == 0)
	{
		shell_fprintf(shell, SHELL_NORMAL, "present DAC setting is %u\n", dac_setting);
	}
	else
	{
		shell_fprintf(shell, SHELL_NORMAL, "Failed to get present DAC setting, error %d\n",
				rc);
	}

	return 0;
}

static int cmd_dac_set_output(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

        uint32_t value = 0;
        char *endptr, *str;
        str = argv[1];
        value = strtol(str, &endptr, BASE_TEN);
	int32_t rc = 0;

	shell_fprintf(shell, SHELL_NORMAL, "to DAC writing value %u . . .\n", value);
	rc = dac_set_output(value);
	return 0;
}

static int cmd_dac_store_setting_for_lock_unlock(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

        uint32_t value = 0;
        char *endptr, *str;
        str = argv[1];
        value = strtol(str, &endptr, BASE_TEN);

	shell_fprintf(shell, SHELL_NORMAL,
		 "storing DAC setting %u for ring lock and unlock operations . . .\n", value);
	ekset_DAC_setting_ring_lock(value);
	return 0;
}

static int cmd_dac_get_setting_for_lock_unlock(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

        uint32_t value = 0;
	ekget_DAC_setting_ring_lock(&value);
	shell_fprintf(shell, SHELL_NORMAL, "present DAC setting for ring lock and unlock is %u\n",
			 value);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
        cmds_dac,
        SHELL_CMD_ARG(range, NULL,
                "show microcontroller DAC range",
                cmd_dac_show_range, 0, 0),
        SHELL_CMD_ARG(show_present_value, NULL,
                "show present DAC setting",
                cmd_dac_show_dac_setting, 0, 0),
        SHELL_CMD_ARG(set, NULL,
                "set DAC output",
                cmd_dac_set_output, 0, 0),
        SHELL_CMD_ARG(set_lock_unlock_current, NULL,
                "store DAC setting for ring lock and unlock",
                cmd_dac_store_setting_for_lock_unlock, 0, 0),
        SHELL_CMD_ARG(show_lock_unlock_current, NULL,
                "show DAC setting for ring lock and unlock",
                cmd_dac_get_setting_for_lock_unlock, 0, 0),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(dac, &cmds_dac, "- ERS - DAC info and set commands", NULL);

//----------------------------------------------------------------------
// - SECTION - init function
//----------------------------------------------------------------------

int32_t ers_init_shell_support(void)
{
	int32_t rc = 0;
	return rc;
}
