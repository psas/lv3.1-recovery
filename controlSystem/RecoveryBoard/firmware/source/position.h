#ifndef _POSITION_H_
#define _POSITION_H_

#include "ch.h"
#include "hal.h"


typedef enum {
	HALL_SENSOR_1,
	HALL_SENSOR_2
} hall_sensor_t;

typedef enum {
	SENSOR_STATE_UNKNOWN = 0,
	SENSOR_STATE_OK,
	SENSOR_STATE_UNDER,
	SENSOR_STATE_OVER
} sensor_status_t;

typedef enum {
	RING_POSITION_UNKNOWN = 0,
	RING_POSITION_UNLOCKED,
	RING_POSITION_IN_BETWEEN,
	RING_POSITION_LOCKED
} ring_position_t;

typedef struct {
	int sensor1;
	int sensor2;
	sensor_status_t sensor1_status;
	sensor_status_t sensor2_status;
	ring_position_t ring_position;
} position_state_t;

const char* sensor_status_t_to_str(const sensor_status_t v);
const char* ring_position_t_to_str(const ring_position_t v);
bool drive_motor(const bool lock_mode, const uint16_t duration_ms, const bool check_hall_sensors);


extern THD_WORKING_AREA(waPositionThread, 256);
extern THD_FUNCTION(PositionThread, arg);




#endif
