#ifndef ERS_UTIL_H
#define ERS_UTIL_H

#define SIZE_SHORT_ERS_MESSAGE 64

#define BASE_TEN 10

// Note: following lock mutices macro expects the following:
// - a declaration of int32_t variable named 'rc'.
// - a label named 'done' in the routine where it is used.
// - the 'done' label appears after any call to unlock the given mutex.

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

#define DEV_0909_LED_ON(caller)                              \
	rc = status_led_on();                                \
	if (rc < 0) {                                        \
		LOG_ERR("DEBUG LED ON FAILED (%s)", caller); \
	}
#if 0
	} else { \
		LOG_INF("%s+", caller); \
	}
#endif // 0

#define DEV_0909_LED_OFF(caller)                              \
	rc = status_led_off();                                \
	if (rc < 0) {                                         \
		LOG_ERR("DEBUG LED OFF FAILED (%s)", caller); \
	}
#if 0
	} else { \
		LOG_INF("%s-", caller); \
	}
#endif // 0

#endif // ERS_UTIL_H
