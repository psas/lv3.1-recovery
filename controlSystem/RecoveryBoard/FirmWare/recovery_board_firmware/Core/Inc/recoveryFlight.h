#ifndef FLIGHT_H
#define FLIGHT_H
#include "main.h"

/* directives
****************************************************/
// LA definitions
// absolute values
#define LIN_ACT_ABS_MAX 2580
#define LIN_ACT_ABS_MIN 0
//comparison values
#define LIN_ACT_OFFSET 20
#define LIN_ACT_UNPULLED (LIN_ACT_ABS_MAX - LIN_ACT_OFFSET)
#define LIN_ACT_PULLED (LIN_ACT_ABS_MIN + LIN_ACT_OFFSET)

// DCM definitions
// 9 pulses per round
#define DCM_PULSE 9

/* enum definitions
****************************************************/
// LA values
typedef enum {
	LA_PULL_UNPULLED,
 	LA_PULL_PULLED
} LA_PullState;

typedef enum {
	LA_EXTEND,
	LA_RETRACT,
	LA_STOP
} LA_State;

// DCM values
typedef enum {
	DCM_FORWARD,
	DCM_BACKWARD
} DCM_Dir;

typedef enum {
	DCM_OFF,
	DCM_ON
} DCM_State;

// sensor values
typedef enum {
	SN_LOCK_LOCKED,
	SN_LOCK_MOVING,
	SN_LOCK_UNLOCKED,
	SN_LOCK_UNKOWN
} SN_LockState;

/* functions
****************************************************/
// releases the nose cone
void drogue();

// releases the main chute
void chute(ADC_HandleTypeDef &hadc);

// the function that runs when everything is armed
int waitForAvionics(ADC_HandleTypeDef &hadc);

// checks what state the sensors detect
SN_LockState sensorState();

// function checks if the sensors return a lock
bool checkLock();

// checks the pulled/unpulled status of the linear actuator
LA_PullState linActPos(ADC_HandleTypeDef &hadc);

// reads motor speed
int motorSpeed();

// move motor
void motorDrive(DCM_Dir dir, DCM_State state);

// move LA
void LAWrite(LA_State state);

void speakerFeedback();

#endif
