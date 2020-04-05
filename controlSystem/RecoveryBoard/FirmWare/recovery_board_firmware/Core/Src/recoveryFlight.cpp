#include "recoveryFlight.h"
#include "recoveryUtils.h"


// releases the nose cone
void drogue(){
	// move the motor until we unlock the cone
	while(sensorState() != SN_LOCK_UNLOCKED){
		motorDrive(DCM_FORWARD, DCM_ON);
	}

	// stop the motor
	motorDrive(DCM_BACKWARD, DCM_OFF);
}


// releases the main chute
void chute(ADC_HandleTypeDef &hadc){
	// run drogue again
	drogue();

	// if pulled, halt, otherwise pull the chute
	while(linActPos(hadc) != LA_PULL_PULLED){
		LAWrite(LA_RETRACT);
	}
	if (linActPos(hadc) == LA_PULL_PULLED){
		LAWrite(LA_STOP);
	}

}


// when recovery board is ARMED, this always runs
int waitForAvionics(ADC_HandleTypeDef &hadc){

	// check drogue status
	if (HAL_GPIO_ReadPin(ISO_DROGUE_GPIO_Port, ISO_DROGUE_Pin) == GPIO_PIN_SET){
		drogue();
	}

	// check chute status
	if (HAL_GPIO_ReadPin(ISO_CHUTE_GPIO_Port, ISO_CHUTE_Pin) == GPIO_PIN_SET){
		chute(hadc);
	}


	return 0;
}


/*
 * this function checks what state the sensors detect:
 * Sensor 1 | Sensor 2 | State
 * 	  1		|    1     | LOCKED
 * 	  1		|	 0	   | MOVING
 * 	  0		|    1	   | UNLOCKED
 * 	  0 	| 	 0	   | UNKOWN
 */
SN_LockState sensorState(){
	SN_LockState state;
	GPIO_PinState sensor1;
	GPIO_PinState sensor2;

	// read the pins to gather each sensor state
	sensor1 = HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin);
	sensor2 = HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin);

	if (sensor1 == GPIO_PIN_SET){
		if (sensor2 == GPIO_PIN_SET){
			// sensor1 = 1, sensor2 = 1
			state = SN_LOCK_LOCKED;
		}
		else{
			// sensor1 = 1, sensor2 = 0
			state = SN_LOCK_MOVING;
		}
	}
	else{
		if (sensor2 == GPIO_PIN_SET){
			// sensor1 = 0, sensor2 = 1
			state = SN_LOCK_UNLOCKED;
		}
		else{
			// sensor1 = 0, sensor2 = 0
			state = SN_LOCK_UNKOWN;
		}
	}
	return state;
}


/*
 *  function checks if the sensors return a lock
 *  true if LOCKED,
 *  false otherwise
 */
bool checkLock(){
	SN_LockState state = sensorState();
	if (state == SN_LOCK_LOCKED){
		return true;
	}
	else{
		return false;
	}
}


// checks the pulled/unpulled status of the linear actuator
LA_PullState linActPos(ADC_HandleTypeDef &hadc){
	uint16_t position;
	position = analogRead(hadc);
	if (position < LIN_ACT_PULLED){
		return LA_PULL_PULLED;
	}
	else{
		return LA_PULL_UNPULLED;
	}
}


// moves the motor
void motorDrive(DCM_Dir dir, DCM_State state){
	// start the PWM channel for the DCM Motor
	// 25 KHz, 50% Duty
//	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
//	TIM3->CCR1 = 50;


	// choose direction
	if (dir == DCM_FORWARD){
		HAL_GPIO_WritePin(DCM_DIR_GPIO_Port, DCM_DIR_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(DCM_DIR_GPIO_Port, DCM_DIR_Pin, GPIO_PIN_RESET);
	}

	// turn motor on or off
	if (state == DCM_ON){
		HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_RESET);
	}
}

// reads motor speed
// 9pulse/round
int motorSpeed(){
	uint32_t * input;
	input = TIM2_PWM_InputRead();

	return input[0] / DCM_PULSE;
}

// function to drive the LA
void LAWrite(LA_State state){
	/* The h-bridge is as follows:
	* IN1 | IN2 | State
	*  L  |  L	| STOP
	*  H  |  L  | EXTEND
	*  L  |  H  | RETRACT
	*  H  |  H  | BRAKE
	*/
	GPIO_PinState IN1;
	GPIO_PinState IN2;

	// decide on inputs
	switch(state){

	case LA_EXTEND:
		IN1 = GPIO_PIN_SET;
		IN2 = GPIO_PIN_RESET;
		break;

	case LA_RETRACT:
		IN1 = GPIO_PIN_RESET;
		IN2 = GPIO_PIN_SET;
		break;

	case LA_STOP:
		IN1 = GPIO_PIN_RESET;
		IN2 = GPIO_PIN_RESET;
		break;

	default:
		IN1 = GPIO_PIN_RESET;
		IN2 = GPIO_PIN_RESET;
		break;
	}

	HAL_GPIO_WritePin(LA_IN1_GPIO_Port, LA_IN1_Pin, IN1);
	HAL_GPIO_WritePin(LA_IN2_GPIO_Port, LA_IN2_Pin, IN2);

}
