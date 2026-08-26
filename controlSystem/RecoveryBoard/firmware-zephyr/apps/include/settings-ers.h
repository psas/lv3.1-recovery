#ifndef SETTINGS_ERS_H
#define SETTINGS_ERS_H

#include <stdint.h>

#define KEY_NAME_LOCK_COUNT "lock_count"
#define KEY_NAME_UNLOCK_COUNT "unlock_count"
#define RING_LOCK_EVENT_STARTING_COUNT 0
#define RING_UNLOCK_EVENT_STARTING_COUNT 0

/**
 * @brief
 */

void ers_settings_init(void);

/**
 * @brief
 */

// TODO [ ] Rename this API to 'ers_settings_retrieve':
int32_t retrieve_ers_setting(const char* name, void *val, const uint32_t size);

/**
 * @brief
 */

// TODO [ ] Rename this API to 'ers_settings_store':
int32_t store_ers_setting(const char* name, const void *val, const uint32_t size);

/**
 * @brief Setting API to store a Hall sensor limit.
 *
 * @note This API takes an index to the group of Hall sensors in the rocket
 *  recovery system, and an index to the group of sensor reading limits.  These
 *  limits are taken to define the position of the lock ring to which Hall
 *  sensors are coupled.
 *
 *  Zephyr's settings subsystem API works with string values to refer to a given
 *  value to store or to retrieve.  The ERS Zephyr application implements some
 *  macros to generate Hall limit names in a grudgingly, semi-automated way, but
 *  there are also places where the it's nicer to code a loop to iterate over
 *  sensors and their limits.  The C language doesn't easily support interating
 *  over a group of like data by string based names.  For this reason certain
 *  modules of the ERS app must map numeric indices to string names.  The
 *  details of settings keyname construction are encapsulated in this API.
 *
 * @param . . .
 *
 * @retval . . .
 */

int32_t ers_settings_store_hall_limit(const uint32_t sensor_idx,
				      const uint32_t limit_idx,
				      const void *val,
				      const uint32_t size);

#endif // SETTINGS_ERS_H
