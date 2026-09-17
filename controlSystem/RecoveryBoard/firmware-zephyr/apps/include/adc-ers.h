#ifndef ERS_ADC_H
#define ERS_ADC_H

#include <keeper.h>

/**
 * @brief Initialize the ADC channel reading module.
 *
 * @retval 0 on success.
 * @retval -ENODEV when an ADC channel is reported as not ready.
 * @retval -EINVAL when there's a failure to set up a ready channel.
 * @retval -EFAULT when the ADC thread fails to start.
 */

int32_t adc_init(void);

/**
 * @brief Read a range of the ADC channels, possibly as few as one channel.
 *
 * @note To read a single channel, begin and end indices should be set equal to
 *  one another.
 *
 * @param idx_begin beginning ADC channel index
 * @param idx_end ending ADC channel index
 *
 * @retval zero on success
 * @return negative errno per Zephyr mutex and analog-to-digital APIs.
 */

int32_t adc_read_channels(const enum ers_adc_values idx_begin, const enum ers_adc_values idx_end);

/**
 * @brief Trampoline function to read analog signal of Hall sensor 1 of 2, and
 *  convert this to a reading in millivolts.
 *
 * @param Pointer to a Zephyr shell context.
 */

int32_t cmd_ers_read_adc_in0(const struct shell *shell);

/**
 * @brief Trampoline function to read analog signal of Hall sensor 2 of 2, and
 *  convert this to a reading in millivolts.
 *
 * @param Pointer to a Zephyr shell context.
 */

int32_t cmd_ers_read_adc_in1(const struct shell *shell);

#endif // ERS_ADC_H
