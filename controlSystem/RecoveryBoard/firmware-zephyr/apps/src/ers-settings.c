/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(ers_settings, CONFIG_ERS_SETTINGS_LOG_LEVEL);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define GAMMA_DEFAULT_VAl 0

#define FAIL_MSG "fail (err %d)\n"

#define SECTION_BEGIN_LINE \
        "\n=================================================\n"

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

// --- ZEPHYR 3.7.0 SETTINGS SAMPLE CODE BEGIN ---
/* Default values are assigned to settings values consuments
 * All of them will be overwritten if storage contain proper key-values
 */
uint8_t angle_val;
uint64_t length_val = 100;
uint16_t length_1_val = 40; 
uint32_t length_2_val = 60; 
int32_t voltage_val = -3000;
char source_name_val[6] = "";

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb,
		     void *cb_arg);
int alpha_handle_commit(void);
int alpha_handle_export(int (*cb)(const char *name,
			const void *value, size_t val_len));

/* dynamic main tree handler */
struct settings_handler alph_handler = {
                .name = "alpha",
                .h_get = NULL,
                .h_set = alpha_handle_set,
                .h_commit = alpha_handle_commit,
                .h_export = alpha_handle_export
};

struct direct_immediate_value {
	size_t len;
	void *dest;
	uint8_t fetched;
};

// --- ZEPHYR 3.7.0 SETTINGS SAMPLE CODE END ---

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int alpha_handle_set(const char *name, size_t len, settings_read_cb read_cb,
		  void *cb_arg)
{
	const char *next;
	size_t next_len;
	int rc;

	if (settings_name_steq(name, "angle/1", &next) && !next) {
		if (len != sizeof(angle_val)) {
			return -EINVAL;
		}
		rc = read_cb(cb_arg, &angle_val, sizeof(angle_val));
		LOG_INF("<alpha/angle/1> = %d", angle_val);
		return 0;
	}

	next_len = settings_name_next(name, &next);

	if (!next) {
		return -ENOENT;
	}

	if (!strncmp(name, "length", next_len)) {
		next_len = settings_name_next(name, &next);

		if (!next) {
			rc = read_cb(cb_arg, &length_val, sizeof(length_val));
			LOG_INF("<alpha/length> = %" PRId64, length_val);
			return 0;
		}

		if (!strncmp(next, "1", next_len)) {
			rc = read_cb(cb_arg, &length_1_val,
				     sizeof(length_1_val));
			LOG_INF("<alpha/length/1> = %d", length_1_val);
			return 0;
		}

		if (!strncmp(next, "2", next_len)) {
			rc = read_cb(cb_arg, &length_2_val,
				     sizeof(length_2_val));
			LOG_INF("<alpha/length/2> = %d", length_2_val);
			return 0;
		}

		return -ENOENT;
	}

	return -ENOENT;
}

int alpha_handle_commit(void)
{
	LOG_INF("loading all settings under <alpha> handler is done");
	return 0;
}

int alpha_handle_export(int (*cb)(const char *name,
			       const void *value, size_t val_len))
{
	LOG_INF("export keys under <alpha> handler");
	(void)cb("alpha/angle/1", &angle_val, sizeof(angle_val));
	(void)cb("alpha/length", &length_val, sizeof(length_val));
	(void)cb("alpha/length/1", &length_1_val, sizeof(length_1_val));
	(void)cb("alpha/length/2", &length_2_val, sizeof(length_2_val));

	return 0;
}

static int direct_loader_immediate_value(const char *name, size_t len,
					 settings_read_cb read_cb, void *cb_arg,
					 void *param)
{
	const char *next;
	size_t name_len;
	int rc;
	struct direct_immediate_value *one_value =
					(struct direct_immediate_value *)param;

	name_len = settings_name_next(name, &next);

	if (name_len == 0) {
		if (len == one_value->len) {
			rc = read_cb(cb_arg, one_value->dest, len);
			if (rc >= 0) {
				one_value->fetched = 1;
				LOG_INF("immediate load: OK.\n");
				return 0;
			}

			LOG_ERR(FAIL_MSG, rc);
			return rc;
		}
		return -EINVAL;
	}

	/* other keys aren't served by the callback
	 * Return success in order to skip them
	 * and keep storage processing.
	 */
	return 0;
}

int load_immediate_value(const char *name, void *dest, size_t len)
{
	int rc;
	struct direct_immediate_value dov;

	dov.fetched = 0;
	dov.len = len;
	dov.dest = dest;

	rc = settings_load_subtree_direct(name, direct_loader_immediate_value,
					  (void *)&dov);
	if (rc == 0) {
		if (!dov.fetched) {
			rc = -ENOENT;
		}
	}

	return rc;
}

static void example_without_handler(void)
{
	uint8_t val_u8;
	int rc;

	LOG_INF(SECTION_BEGIN_LINE);
	LOG_INF("Service a key-value pair without dedicated handlers\n\n");
	rc = load_immediate_value("gamma", &val_u8, sizeof(val_u8));
	if (rc == -ENOENT) {
		val_u8 = GAMMA_DEFAULT_VAl;
		LOG_WRN("<gamma> = %d (default)\n", val_u8);
	} else if (rc == 0) {
		LOG_INF("<gamma> = %d\n", val_u8);
	} else {
		LOG_ERR("Failed to load immediate value, err %d", rc);
	}

	val_u8++;

	LOG_INF("save <gamma> key directly: ");
	rc = settings_save_one("gamma", (const void *)&val_u8,
			       sizeof(val_u8));
	if (rc) {
		LOG_ERR(FAIL_MSG, rc);
	} else {
		LOG_INF("OK.\n");
	}
}

void ers_settings_init(void)
{
	int32_t rc;

	rc = settings_subsys_init();
	if (rc) {
		LOG_ERR("Failed to initialize settings subsystem, err %d", rc);
		return;
	}

	LOG_INF("settings subsystem initialization OK.");

	rc = settings_register(&alph_handler);
	if (rc) {
		LOG_ERR("Failed to register subtree <%s> handler, err %d",
		       alph_handler.name, rc);
	}

	LOG_INF("subtree <%s> handler registered: OK", alph_handler.name);
	LOG_INF("subtree <alpha/beta> has static handler");

	uint32_t i;
	for (i = 0; i < 6; i++) {

		LOG_INF("settings exercise iteration %u:", i);
		/*---------------------------------------
		 * a key-value without dedicated handler
		 */
		example_without_handler();
	}
}
