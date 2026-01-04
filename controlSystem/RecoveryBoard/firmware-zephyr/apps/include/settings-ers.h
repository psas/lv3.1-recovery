#ifndef SETTINGS_ERS_H
#define SETTINGS_ERS_H

#define KEY_NAME_LOCK_COUNT "lock_count"
#define KEY_NAME_UNLOCK_COUNT "unlock_count"
#define RING_LOCK_EVENT_STARTING_COUNT 0
#define RING_UNLOCK_EVENT_STARTING_COUNT 0

void ers_settings_init(void);

int32_t retrieve_ers_setting(const char* name, void *val, const uint32_t size);

int32_t store_ers_setting(const char* name, const void *val, const uint32_t size);

#endif // SETTINGS_ERS_H
