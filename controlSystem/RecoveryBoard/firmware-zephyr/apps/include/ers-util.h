#ifndef ERS_UTIL_H
#define ERS_UTIL_H

#define SIZE_SHORT_ERS_MESSAGE 64

#define BASE_TEN 10

#define ERS_MUTEX_LOCK(mtx, timeout, module_name)                                       \
        rc = k_mutex_lock(&mtx, K_MSEC(timeout));                                       \
        if (rc < 0) {                                                                   \
                LOG_ERR("Failed to lock %s mutex, err %d", STRINGIFY(module_name), rc); \
                goto done;                                                              \
        }

#define ERS_MUTEX_UNLOCK(mtx, module_name)                                                \
        rc = k_mutex_unlock(&mtx);                                                        \
        if (rc < 0) {                                                                     \
                LOG_ERR("Failed to unlock %s mutex, err %d", STRINGIFY(module_name), rc); \
        }

#endif // ERS_UTIL_H
