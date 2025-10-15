#ifndef ERS_KEEPER_H
#define ERS_KEEPER_H

// Note, next two enumeration names refer to ADC values and not channels, as
// two values per active channel are stored:  (1) ADC counts and (2) reading
// converted to millivolts.

// Note, these enum values are used in a switch construct, so that they serve as
// something like an index.

enum ers_adc_values {
        ADC_READING_HALL_1,        // ADC_IN0
        ADC_READING_HALL_2,        // ADC_IN1
        ADC_READING_BATT_READ,     // ADC_IN8
        ADC_READING_MOTOR_ISENSE,  // ADC_IN9
        ADC_CHANNEL_COUNT
};

enum ers_adc_values_in_mv {
        ADC_READING_HALL_1_MV,
        ADC_READING_HALL_2_MV,
        ADC_READING_BATT_READ_MV,
        ADC_READING_MOTOR_ISENSE_MV
};

#define IDX_START_MV_READINGS ADC_CHANNEL_COUNT

// Module init API

int32_t ers_init_keeper(void);

// Digital inputs

void ekset_iso_drogue(const uint32_t value);
void ekset_iso_main(const uint32_t value);
void ekset_not_umb_on(const uint32_t value);
void ekset_not_motor_faila(const uint32_t value);

void ekget_iso_drogue(uint32_t* value);
void ekget_iso_main(uint32_t* value);
void ekget_not_umb_on(uint32_t* value);
void ekget_not_motor_faila(uint32_t* value);

// Analog inputs

void ekset_batt_read(const uint32_t value);
void ekset_motor_isense(const uint32_t value);
void ekset_hall_1(const uint32_t value);
void ekset_hall_2(const uint32_t value);

void ekset_batt_read_mv(const uint32_t value);
void ekset_motor_isense_mv(const uint32_t value);
void ekset_hall_1_mv(const uint32_t value);
void ekset_hall_2_mv(const uint32_t value);

void ekset_batt_read_dv(const uint32_t value);

/**
 * @return 0 on success to obtain mutex and to set sensor values
 * @return -ESRCH when module not initialized
 */

int32_t ekset_both_hall_sensors(const uint32_t value_1, const uint32_t value_2);

/**
 * @brief API to set any one of ADC channel readings
 */

int32_t ekset_adc_value(const enum ers_adc_values idx, const uint32_t value);

/**
 * @brief API to set any one of ADC channel readings converted to millivolts
 */

int32_t ekset_adc_value_in_mv(const enum ers_adc_values_in_mv idx, const uint32_t val);

void ekget_batt_read(uint32_t* value);
void ekget_motor_isense(uint32_t* value);
void ekget_hall_1(uint32_t* value);
void ekget_hall_2(uint32_t* value);

void ekget_batt_read_mv(uint32_t* value);
void ekget_motor_isense_mv(uint32_t* value);
void ekget_hall_1_mv(uint32_t* value);
void ekget_hall_2_mv(uint32_t* value);

void ekget_batt_read_dv(uint32_t* value);

/**
 * @return 0 on success to obtain mutex and to get sensor values
 * @return -ESRCH when module not initialized
 */
int32_t ekget_both_hall_sensors(uint32_t *value_1, uint32_t *value_2);

// Off-chip peripherals and system statae

void ekset_ring_status(const uint32_t value);
void ekset_batt_ok(const uint32_t value);
void ekset_shore_power_ok(const uint32_t value);
void ekset_can_bus_ok(const uint32_t value);
void ekset_ready_state(const uint32_t value);

void ekget_ring_status(uint32_t* value);
void ekget_batt_ok(uint32_t* value);
void ekget_shore_power_ok(uint32_t* value);
void ekget_can_bus_ok(uint32_t* value);
void ekget_ready_state(uint32_t* value);

// ERS diagnostics

void ek_sys_diag_periodic(void);
void ek_sys_diag_quiet(void);
void ek_get_sys_diag_mode(uint32_t* value);

#endif // ERS_KEEPER_H
