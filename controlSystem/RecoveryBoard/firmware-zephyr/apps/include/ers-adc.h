#ifndef ERS_ADC_H
#define ERS_ADC_H

#include <keeper.h>

int32_t adc_init(void);

int32_t adc_read_channels(const enum ers_adc_values idx_begin, const enum ers_adc_values idx_end);

#endif // ERS_ADC_H
