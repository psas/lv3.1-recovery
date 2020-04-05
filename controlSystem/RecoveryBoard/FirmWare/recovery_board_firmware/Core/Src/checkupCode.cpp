#include "checkupCode.h"
#include "recoveryUtils.h"
#include "main.h"

// test the LED
void ledBlink(){
	int i=0;


	while(i<5){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(500);
	i++;
	}
}

// test the speaker
void spkrTest(){
	int i=0;
	// turn on speaker
	HAL_GPIO_WritePin(SPKR_GPIO_Port, SPKR_Pin, GPIO_PIN_SET);
	HAL_Delay(2000);

	// LED flash to confirm moving to next test
	while(i<4){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(500);
	i++;
	}

}

// test the motor
void motorTest(){
	int i=0;
	int raw;
	// change the direction of the motor
	HAL_GPIO_WritePin(DCM_DIR_GPIO_Port, DCM_DIR_Pin, GPIO_PIN_RESET);

	// Set the motor pin to on
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_SET);
	HAL_Delay(5000);

	// stop motor
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
	// restart motor
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_SET);

	// change the direction of the motor
	HAL_GPIO_WritePin(DCM_DIR_GPIO_Port, DCM_DIR_Pin, GPIO_PIN_SET);
	HAL_Delay(5000);

	// turn motor off
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_RESET);



	// LED flash to confirm moving to next test
	while(i<6){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(500);
	i++;
	}
}

// function to make running the H Bridge easier
void writeLA(int INPUT1, int INPUT2){
	GPIO_PinState IN1;
	GPIO_PinState IN2;

	if(INPUT1){
		IN1 = GPIO_PIN_SET;
	}
	else{
		IN1 = GPIO_PIN_RESET;
	}
	if(INPUT2){
		IN2 = GPIO_PIN_SET;
	}
	else{
		IN2 = GPIO_PIN_RESET;
	}

	HAL_GPIO_WritePin(LA_IN1_GPIO_Port, LA_IN1_Pin, IN1);
	HAL_GPIO_WritePin(LA_IN2_GPIO_Port, LA_IN2_Pin, IN2);

}


//test the linear actuator
void testLA(ADC_HandleTypeDef &hadc){
	int i=0;
	int raw;
	// The h-bridge is as follows:
	// 1 2
	// L L STOP
	// H L FORWARD
	// L H REVERSE
	// H H BRAKE

	// stop
	writeLA(0, 0);
	HAL_Delay(1000);

	// forward
	writeLA(1, 0);
	raw = analogRead(hadc);

	// halt
	writeLA(0, 0);
	raw = analogRead(hadc);

	// back
	writeLA(0, 1);
	raw = analogRead(hadc);
	writeLA(0, 0);
	// forward again
	raw = analogRead(hadc);
	writeLA(1, 0);
	raw = analogRead(hadc);
	// halt
	writeLA(0, 0);
	raw = analogRead(hadc);
	// backward again

	writeLA(0, 1);
	raw = analogRead(hadc);

	// brake
	writeLA(1, 1);
	raw = analogRead(hadc);



	// there's also the potentiometer that needs to be tested


	// LED flash to confirm moving to next test
	while(i<8){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(500);
	i++;
	}
}


// test one sensor
void sensorTestSingle(int sensor){
	// turn sensors on
	HAL_GPIO_WritePin(SENSOR_ON_GPIO_Port, SENSOR_ON_Pin, GPIO_PIN_SET);

	// allows us to test either sensor one at a time
	if(sensor == 1){

		if (HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin) == GPIO_PIN_SET){
			// when the sensor detects HIGH, make the LED blink
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		}
		else{
			// turn off any other time
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}
	}
	else{
		// testing sensor 2 now
		if (HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin) == GPIO_PIN_SET){
			// when the sensor detects HIGH, make the LED blink
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		}
		else{
			// turn off any other time
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}
	}
}


// test the sensors and their reading
void sensorTestDouble(){
	int sensor1;
	int sensor2;

	// turn sensors on
	HAL_GPIO_WritePin(SENSOR_ON_GPIO_Port, SENSOR_ON_Pin, GPIO_PIN_SET);
	sensor1 = HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin);
	sensor2 = HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin);
	// allows us to test either sensor one at a time
	if (sensor1 == GPIO_PIN_SET){
		// when the sensor detects HIGH, make the LED blink
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
	else{
		// turn off any other time
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}		// testing sensor 2 now
	if (sensor2 == GPIO_PIN_SET){
		// when the sensor detects HIGH, make the LED blink
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
	else{
		// turn off any other time
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
}

// reading the telemetrum signals
void telemetrumTest(){
	int i = 0;
	GPIO_PinState valueChute;
	GPIO_PinState valueDrogue;

	valueChute = HAL_GPIO_ReadPin(ISO_CHUTE_GPIO_Port, ISO_CHUTE_Pin);
	if(valueChute == GPIO_PIN_SET){
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
	else{
	// turn off any other time
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
	valueDrogue = HAL_GPIO_ReadPin(ISO_DROGUE_GPIO_Port, ISO_DROGUE_Pin);
	if(valueDrogue == GPIO_PIN_SET){
		while(i<2){
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_Delay(100);
		i++;
		}
	}
	else{
	// turn off any other time
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
}

// full test routine
void testRoutine(){
	// running through the above test protocols
}
