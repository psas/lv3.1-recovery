#ifndef SETTINGS_ERS_H
#define SETTINGS_ERS_H

void ers_settings_init(void);

int32_t retrieve_ers_setting(const char* name, void *val, const uint32_t size);

int32_t store_ers_setting(const char* name, const void *val, const uint32_t size);

#endif // SETTINGS_ERS_H
