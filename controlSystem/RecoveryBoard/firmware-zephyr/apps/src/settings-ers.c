/*
 * @file
 *
 * @brief ERS module which uses Zephyr's settings sub-system to write and to
 *  read values using flash memory as a non-volatile store.
 *
 * @note Comments in this app use 'store' and 'retrieve' to refer to flash
 *  write and flash read operations.
 */

#include "hall-and-ring.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <stdio.h>
#include <string.h>

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

static int load_immediate_value(const char *name, void *dest, size_t len)
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

int32_t settings_ers_retrieve_value(const char* name, void *val, const uint32_t size)
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

int32_t settings_ers_store_value(const char* name, const void *val, const uint32_t size)
{
	int32_t rc = 0;

	LOG_INF("save '%s' key directly: ", name);
	rc = settings_save_one(name, &val, sizeof(val));
	if (rc < 0) {
		LOG_ERR("Fail to store value for '%s', err %d", name, rc);
	}
	return rc;
}

// Helper enum to allow single switch statement to select its case based on
// both Hall sensor instance, and Hall reading cut-off value:

enum sensor_plus_limit_enum {
	HALL_1_LIMIT_1 = (0 << 4) + 0,
	HALL_1_LIMIT_2 = (0 << 4) + 1,
	HALL_1_LIMIT_3 = (0 << 4) + 2,
	HALL_1_LIMIT_4 = (0 << 4) + 3,
	HALL_2_LIMIT_1 = (1 << 4) + 0,
	HALL_2_LIMIT_2 = (1 << 4) + 1,
	HALL_2_LIMIT_3 = (1 << 4) + 2,
	HALL_2_LIMIT_4 = (1 << 4) + 3,
};

int32_t settings_ers_store_hall_limit(const uint32_t sensor_idx,
				const uint32_t limit_idx,
				const void *val,
				const uint32_t size)
{
	int32_t rc = 0;

	if (sensor_idx >= HALL_SENSOR_COUNT) {
		LOG_ERR("Hall sensor index %d out of range, must be in 0..%d",
				sensor_idx, (HALL_SENSOR_COUNT - 1));
		rc = -EINVAL;
		goto done;
	}

	if (limit_idx >= HALL_SENSOR_LIMIT_COUNT) {
		LOG_ERR("Sensor limit index %d out of range, must be in 0..%d",
				sensor_idx, (HALL_SENSOR_LIMIT_COUNT - 1));
		rc = -EINVAL;
		goto done;
	}

	uint32_t combined_idx = (sensor_idx << 4) + limit_idx;

	switch (combined_idx)
	{
	case HALL_1_LIMIT_1:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_1),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_1_LIMIT_2:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_2),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_1_LIMIT_3:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_3),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_1_LIMIT_4:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S1_HLIMIT_4),
				 	(const void *)val, sizeof(val));
		break;

	case HALL_2_LIMIT_1:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_1),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_2_LIMIT_2:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_2),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_2_LIMIT_3:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_3),
				 	(const void *)val, sizeof(val));
		break;
	case HALL_2_LIMIT_4:
		rc = settings_ers_store_value(STRINGIFY(SETTING_KEYNAME_S2_HLIMIT_4),
				 	(const void *)val, sizeof(val));
		break;

	default:
		LOG_ERR("Got undefined sensor and limit combination %d in store API.",
			combined_idx);
	}
done:
	return rc;
}

void settings_ers_init(void)
{
	int32_t rc;

	rc = settings_subsys_init();
	if (rc) {
		LOG_ERR("Failed to initialize settings subsystem, err %d", rc);
		return;
	}

	LOG_INF("settings subsystem initialization OK.");
}
