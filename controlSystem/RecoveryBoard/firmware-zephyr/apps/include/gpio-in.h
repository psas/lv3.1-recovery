#ifndef GPIO_IN_H
#define GPIO_IN_H

/**
 * @brief Initialize ERS GPIO pin module.
 *
 * @retval 0 on success.
 * @return negative errno from one of pin or interrupt configuration Zephyr
 *  APIs.
 */

int32_t gpio_in_init(void);

#endif // GPIO_IN_H
