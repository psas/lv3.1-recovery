#ifndef _POSITION_H_
#define _POSITION_H_

#include "ch.h"
#include "hal.h"


typedef enum {
	HALL_SENSOR_1,
	HALL_SENSOR_2
} hall_sensor_t;

typedef enum {
	SENSOR_VOLTAGE_STATUS_UNKNOWN = 0,
	SENSOR_VOLTAGE_STATUS_OK,
	SENSOR_VOLTAGE_STATUS_UNDER_VOLTAGE,
	SENSOR_VOLTAGE_STATUS_OVER_VOLTAGE
} sensor_voltage_status_t;

typedef enum {
	RING_POSITION_UNKNOWN = 0,
	RING_POSITION_UNLOCKED,
	RING_POSITION_IN_BETWEEN,
	RING_POSITION_LOCKED
} ring_position_t;

typedef struct {
	int sensor1;
	int sensor2;
	sensor_voltage_status_t sensor1_voltage_status;
	sensor_voltage_status_t sensor2_voltage_status;
	ring_position_t ring_position;
} position_state_t;

const char* sensor_voltage_status_t_to_str(const sensor_voltage_status_t v);
const char* ring_position_t_to_str(const ring_position_t v);
bool drive_motor(const bool lock_mode, const uint16_t duration_ms);


extern THD_WORKING_AREA(waPositionThread, 512);
extern THD_FUNCTION(PositionThread, arg);




#endif
