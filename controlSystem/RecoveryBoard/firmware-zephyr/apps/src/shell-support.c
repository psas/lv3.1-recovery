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

#define SHELL_SUPPORT_THREAD_STACK_SIZE 512
#define SHELL_SUPPORT_THREAD_PRIORITY 5

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

static const struct shell *shell_ptr_fs = NULL;

static uint32_t dev_test_calls_fs = 0;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

#if 0
static int ers_cmd_wrapper_read_adc_all(const struct shell *shell, size_t argc, char *argv[])
{
	int rc = cmd_ers_read_adc_all(shell);
	return rc;
}
#endif

static int ers_cmd_wrapper_read_adc_in0(const struct shell *shell, size_t argc, char *argv[])
{
	int32_t rc = 0;
	shell_fprintf(shell, SHELL_NORMAL, "- STUB FUNCTION - to read and print ADC IN0 channel.\n");
	return rc;
}

static int ers_cmd_print_mark(const struct shell *shell, size_t argc, char *argv[])
{
	shell_fprintf(shell, SHELL_NORMAL, "- MARK -\n");
        shell_ptr_fs = shell;
	return 0;
}

static int ers_cmd_print_shell_addr(const struct shell *shell, size_t argc, char *argv[])
{
	shell_fprintf(shell, SHELL_NORMAL, "Current shell at addr 0x%" PRIxPTR "\n",
		      (long unsigned int)shell_ptr_fs);
	shell_fprintf(shell, SHELL_NORMAL, "DEV 0926 app shell print test called %u "
		      " times\n", dev_test_calls_fs);
	return 0;
}

//----------------------------------------------------------------------
// - SECTION - ERS diagnotics
//----------------------------------------------------------------------

static int ers_cmd_diag_periodic_on(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	ek_sys_diag_periodic();
	return 0;
}

static int ers_cmd_diag_periodic_off(const struct shell *shell, size_t argc, char *argv[])
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
		ers_cmd_diag_periodic_on, 0, 0),
	SHELL_CMD_ARG(off, NULL,
		"disable ERS periodic diagnostics",
		ers_cmd_diag_periodic_off, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(diag, &ers_cmds_diag, "- ERS - diagnostics", NULL);

//----------------------------------------------------------------------
// - SECTION - ERS ADC commands
//----------------------------------------------------------------------

static int ers_cmd_wrapper_read_adc_all(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	int32_t rc = 0;

        shell_print(shell, "Calling ADC module to read all ADC channels . . .");
	rc = adc_read_channels(ADC_READING_BATT_READ, ADC_READING_HALL_2);
	if (rc != 0)
	{
		LOG_ERR("ADC read channels returns error status %d", rc);
		LOG_ERR("Last known good stored readings are:");
	}

	uint32_t a, b, c, d;
	ekget_batt_read(&a);
	ekget_motor_isense(&b);
	ekget_hall_1(&c);
	ekget_hall_2(&d);
	LOG_INF("ADC counts for batter, motor current, Hall 1, Hall 2:");
	LOG_INF("%u  %u  %u  %u", a, b, c, d);

	return rc;
}

// TODO [ ] move all command routines referenced by this Zephyr macro to this
//   in-file section.

SHELL_STATIC_SUBCMD_SET_CREATE(
	ers_cmds,
	SHELL_CMD_ARG(adcall, NULL,
		"Read ERS board's four ADC channels",

		ers_cmd_wrapper_read_adc_all, 0, 0),
	SHELL_CMD_ARG(adc0, NULL,
		"Read ERS board ADC for Hall sensor 1",
		ers_cmd_wrapper_read_adc_in0, 0, 0),
#if 0
	SHELL_CMD_ARG(adc1, NULL,
		"Read ERS board ADC for Hall sensor 2",
		cmd_ers_read_adc_in1, 0, 0),
#endif
	SHELL_CMD_ARG(print_mark, NULL,
		"output to shell console a brief 'mark' message",
		ers_cmd_print_mark, 0, 0),
	SHELL_CMD_ARG(print_shell_ptr, NULL,
		"print address of run time Zephyr shell instance",
		ers_cmd_print_shell_addr, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(ers, &ers_cmds, "- ERS - development commands", NULL);

//----------------------------------------------------------------------
// - SECTION - ERS lock ring commands (IN PROGRESS)
//----------------------------------------------------------------------

static int sw_show_locking_ring_pos(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	enum lock_ring_position ring_position = RING_POSITION_UNKNOWN;
	int32_t rc = arbiter_determine_ring_state(&ring_position);
	if (rc == 0)
	{
		char lbuf[SIZE_SHORT_ERS_MESSAGE] = {0};
		char *ring_pos_as_str = lbuf;
		ring_pos_as_str = ring_pos_to_str(ring_position);
		LOG_INF("Current lock ring position:  %d %s", ring_position, ring_pos_as_str);
	}
	else
	{
		LOG_INF("Failed lock ring position query, error %d", rc);
	}

	return rc;
}

static int sw_lock_ring(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	LOG_INF("- STUB - for command to lock parachute locking ring");
	return 0;
}

static int sw_unlock_ring(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);
	LOG_INF("- STUB - for command to lock parachute locking ring");
	return 0;
}

SHELL_SUBCMD_SET_CREATE(sub_section_ring, (ring));

SHELL_SUBCMD_ADD((ring), show_position, &sub_section_ring, "show locking ring position", sw_show_locking_ring_pos, 1, 0);
// TODO [ ] add command to lock ring
// TODO [ ] add command to unlock ring
SHELL_SUBCMD_ADD((ring), lock, &sub_section_ring, "lock ring", sw_lock_ring, 1, 0);

SHELL_SUBCMD_ADD((ring), unlock, &sub_section_ring, "unlock ring", sw_unlock_ring, 1, 0);

SHELL_CMD_REGISTER(ring, &sub_section_ring, "- ERS - lock ring commands", NULL);

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

SHELL_SUBCMD_ADD((hall), show_cutoffs, &sub_section_hall, "(1) show Hall state cutoff values", arbiter_show_hall_state_cutoffs, 1, 0);

SHELL_SUBCMD_ADD((hall), v_under_cutoff, &sub_section_hall_set, "(2) set Hall state voltage under cutoff", sw_set_v_under_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), inactive_cutoff, &sub_section_hall_set, "(3) set Hall state inactive cutoff", sw_set_inactive_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), between_cutoff, &sub_section_hall_set, "(4) set Hall state between cutoff", sw_set_between_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), active_cutoff, &sub_section_hall_set, "(5) set Hall state active cutoff", sw_set_active_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), defaults, &sub_section_hall, "(6) set Hall cutoff default values", sw_set_default_cutoffs, 1, 0);

SHELL_CMD_REGISTER(hall, &sub_section_hall, "- ERS - set and show Hall state cutoff values (in ADC counts)", NULL);

//----------------------------------------------------------------------
// - SECTION - DAC commands
//----------------------------------------------------------------------

static int cmd_dac_show_range(const struct shell *shell, size_t argc, char *argv[])
{
	// LOG_INF("stub command to show DAC range");
	uint32_t bound_low = 0;
	uint32_t bound_high = 0;
	int32_t rc = dac_range(&bound_low, &bound_high);
	LOG_INF("DAC range is %u..%u", bound_low, bound_high);
	return 0;
}

static int cmd_dac_show_dac_setting(const struct shell *shell, size_t argc, char *argv[])
{
	// LOG_INF("stub command to show present DAC setting");
	uint32_t dac_setting = 0;
	int32_t rc = dac_present_value(&dac_setting);
	if (rc == 0)
	{
		LOG_INF("present DAC setting is %u", dac_setting);
	}
	else
	{
		LOG_ERR("Failed to get present DAC setting, error %d", rc);
	}

	return 0;
}

static int cmd_dac_set_output(const struct shell *shell, size_t argc, char *argv[])
{
	LOG_INF("stub command to set DAC output");

        uint32_t value = 0;
        char *endptr, *str;
        str = argv[1];
        value = strtol(str, &endptr, 10);  // TODO [ ] factor BASE_10 symbol to ers-utils.h header and use it here
	int32_t rc = 0;

	rc = dac_set_output(value);
	return 0;
}

// The following creates commands:
//
//   uart$ dac range
//   uart$ dac show_present_value
//   uart$ dac set <value>

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
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(dac, &cmds_dac, "- ERS - DAC info and set commands", NULL);

int32_t ers_init_shell_support(void)
{
	int32_t rc = 0;
	return rc;
}

#if 0
//======================================================================
// DEV CODE BEGIN
//
// This code added to see if we can define commands beyond a depth of
// two.  Maybe not important, less easy to type such commands, but we
// also face something of a flat command name space with only two levels
// of command depth.  This so given Zephyr shell's default commands
// which are present in its shell module and don't have an obvious way
// to be disabled.

static int cmd1_handler(const struct shell *sh, size_t argc, char **argv)
{
        shell_print(sh, "cmd1 executed");
        return 0;
}

static int cmd2_handler(const struct shell *sh, size_t argc, char **argv)
{
        shell_print(sh, "cmd2 executed");
        return 0;
}

// (1)
SHELL_SUBCMD_SET_CREATE(sub_section_cmd, (section_cmd));

// (2)
/* Create a set of subcommands for "section_cmd cm1". */
// SHELL_SUBCMD_SET_CREATE(sub_section_cmd1, (section_cmd, cmd1));
SHELL_SUBCMD_SET_CREATE(sub_section_cmd1, (section_cmd, cmd1, cmd2));

// (3)
/* Add command to the set. Subcommand set is identify by parent shell command. */
SHELL_SUBCMD_ADD((section_cmd), cmd1, &sub_section_cmd1, "help for cmd1", cmd1_handler, 1, 0); 

SHELL_SUBCMD_ADD((section_cmd), cmd2, &sub_section_cmd1, "help for cmd2", cmd2_handler, 1, 0); 

// (4)
SHELL_CMD_REGISTER(section_cmd, &sub_section_cmd,
                   "Demo command using section for subcommand registration", NULL);

// DEV CODE END
//======================================================================
#endif // 0

