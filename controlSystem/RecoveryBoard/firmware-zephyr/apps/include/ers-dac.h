#ifndef ERS_DAC_H
#define ERS_DAC_H

int32_t ers_init_dac(void);

int32_t dac_set_output(const uint32_t value);

int32_t dac_present_value(uint32_t *dac_setting);

int32_t dac_range(int32_t *bound_low, int32_t *bound_high);

#endif // ERS_DAC_H
