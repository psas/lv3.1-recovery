/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(ers_settings, CONFIG_ERS_SETTINGS_LOG_LEVEL);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define GAMMA_DEFAULT_VAl 0

#define FAIL_MSG "fail (err %d)"

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

// Note following structure borrowed from Zephyr 3.7.0 settings sample code:

struct direct_immediate_value {
	size_t len;
	void *dest;
	uint8_t fetched;
};

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

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
				LOG_INF("immediate load: OK.");
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

/**
 * @brief Retrieve a persistent setting from the key named by parameter 'name'.
 *
 * @note This pair of 'store' and 'retrieve' routines provides a general purpose
 *  setter/getter API pair to MCU flash.  Callers provide the Zephyr "setting"
 *  string wise name, a key name, and this module averts the need to implement
 *  some multiple store and retrieve setting functions which differ only in
 *  setting key name.
 */

int32_t retrieve_ers_setting(const char* name, void *val, const uint32_t size)
{
	int32_t rc = 0;

	rc = load_immediate_value(name, val, size);
	if (rc == -ENOENT) {
		// LOG_ERR("Key '%s' not yet initialized, read status %d", name, rc);
	} else if (rc < 0) {
		LOG_ERR("Failed to load immediate value, err %d", rc);
	}
	return rc;
}

int32_t store_ers_setting(const char* name, const void *val, const uint32_t size)
{
	int32_t rc = 0;

	LOG_INF("save '%s' key directly: ", name);
	rc = settings_save_one(name, &val, sizeof(val));
	// if (rc) {
	if (rc < 0) {
		LOG_ERR("Fail to store value for '%s', err %d", name, rc);
	}
	return rc;
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
}
