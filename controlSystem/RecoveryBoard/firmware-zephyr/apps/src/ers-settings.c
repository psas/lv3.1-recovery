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
}
