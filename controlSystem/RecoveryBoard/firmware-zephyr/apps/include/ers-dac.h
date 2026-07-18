#ifndef ERS_DAC_H
#define ERS_DAC_H

int32_t ers_init_dac(void);

/**
 * @brief Routine to write DAC output register, to set and to possibly change
 *  the digital-to-analog converter's output.
 * @param value Positive integer value representing DAC output.
 * @retval 0 on success.
 * @retval -ENODEV when DAC device not initialized.
 * @retval -EINVAL when DAC output outside valid integral range.
 * @return negative errno as produced by dac_write_value().
 */

int32_t dac_write_output_reg(const uint32_t value);

int32_t dac_present_value(uint32_t *dac_setting);

int32_t dac_range(int32_t *bound_low, int32_t *bound_high);

#endif // ERS_DAC_H
