/**
 * @file
 * @brief ERS Zephyr app, motor control module header
 */

/**
 * @brief Initialize the motor control module.
 *
 * @retval 0 on success.
 * @return -errno from one of GPIO configure functions, or from settings API
 *  calls.
 */

int32_t ers_init_motor_ctrl(void);

/**
 * @brief Lock the parachute airframe section ring.
 *
 * @retval 0 on success.
 * @return -errno from one of motor drive chip control line functions.
 */

int32_t mc_lock_ring(void);

/**
 * @brief Unlock the parachute airframe section ring.
 *
 * @retval 0 on success.
 * @return -errno from one of motor drive chip control line functions.
 */

int32_t mc_unlock_ring(void);
