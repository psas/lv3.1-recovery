#ifndef SETTINGS_ERS_H
#define SETTINGS_ERS_H

#include <stdint.h>

#define KEY_NAME_LOCK_COUNT "lock_count"
#define KEY_NAME_UNLOCK_COUNT "unlock_count"
#define RING_LOCK_EVENT_STARTING_COUNT 0
#define RING_UNLOCK_EVENT_STARTING_COUNT 0

/**
 * @brief Initialize ERS persistent data management module.
 */

void settings_ers_init(void);

/**
 * @brief Retrieve one or more bytes, identifying them  with a named key.
 *
 * @pararm name Keyname to search in store of settings.
 * @pararm val Pointer to caller variable to hold retrieved value.
 * @pararm size Expected count of bytes in value.
 *
 * @retval 0 on success.
 * @return non-zero per Zephyr's settings_load_subtree_direct() API.
 */

int32_t settings_ers_retrieve_value(const char* name, void *val, const uint32_t size);

/**
 * @brief Save one or more bytes and associate them with a named key.
 *
 * @pararm name Keyname to store with the value to be stored.
 * @pararm val Pointer to caller variable which holds value to store.
 * @pararm size Expected count of bytes in value.
 *
 * @retval 0 on success.
 * @return non-zero per Zephyr's settings_save_one() API.
 */

int32_t settings_ers_store_value(const char* name, const void *val, const uint32_t size);

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
 * @param sensor_idx Hall sensor ID per enum of system's two Hall sensors.
 * @param limit_idx Hall reading cut-off identifier, a named cut-off or "limit".
 * @param val Value of the reading considered the state wise cut-off point.
 * @param size Size of the value in bytes.
 *
 * @retval 0 on success.
 * @return non-zero per Zephyr's settings_save_one() API.
 */

int32_t settings_ers_store_hall_limit(const uint32_t sensor_idx,
				      const uint32_t limit_idx,
				      const void *val,
				      const uint32_t size);

#endif // SETTINGS_ERS_H
