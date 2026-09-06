#ifndef ERS_CAN_INIT
#define ERS_CAN_INIT

/**
 * @brief Initialize ERS Zephyr CAN module
 *
 * @retval -ENODEV when physical CAN bus is reported not ready.
 * @retval -EAGAIN when CAN bus fails to start.
 * @retval -ESRCH when module fails to create thread for CAN message handling.
 */

int32_t ers_can_init(void);

#endif // ERS_CAN_INIT
