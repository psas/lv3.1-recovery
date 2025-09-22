#ifndef _POSITION_H_
#define _POSITION_H_

#include "ch.h"
#include "hal.h"

typedef enum { HALL_SENSOR_1, HALL_SENSOR_2 } hall_sensor_t;

typedef enum {
  SENSOR_STATE_ACTIVE = 0,
  SENSOR_STATE_UNACTIVE,
  SENSOR_STATE_UNDER,
  SENSOR_STATE_OVER,
  SENSOR_STATE_INBETWEEN,
  SENSOR_STATE_ERROR
} sensor_state_t;

typedef enum {
  RING_POSITION_UNLOCKED = 0,
  RING_POSITION_LOCKED,
  RING_POSITION_INBETWEEN,
  RING_POSITION_ERROR
} ring_position_t;

typedef struct {
  int sensor1;
  int sensor2;
  sensor_state_t sensor1_state;
  sensor_state_t sensor2_state;
  ring_position_t ring_position;
} position_state_t;

const char* sensor_state_t_to_str(const sensor_state_t sensor_state);
const char* ring_position_t_to_str(const ring_position_t ring_pos);
bool drive_motor(const bool lock_mode,
                 const uint16_t duration_ms,
                 const bool check_ring_position,
                 const uint16_t current);

extern THD_WORKING_AREA(waPositionThread, 256);
extern THD_FUNCTION(PositionThread, arg);

#endif
