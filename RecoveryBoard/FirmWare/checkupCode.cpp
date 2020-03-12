#include "checkupCode.h"
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
	HAL_GPIO_TogglePin(SPKR_GPIO_Port, SPKR_Pin);
	HAL_Delay(2000);

	// LED flash to confirm moving to next test
	while(i<2){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(1000);
	i++;
	}

}

// test the motor
void motorTest(){
	int i=0;
	// Set the motor pin to on
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_SET);
	HAL_Delay(5000);

	// change the direction of the motor
	HAL_GPIO_WritePin(DCMDIR_GPIO_Port, DCMDIR_Pin, GPIO_PIN_SET);
	HAL_Delay(5000);

	// turn motor off
	HAL_GPIO_WritePin(DCM_ON_GPIO_Port, DCM_ON_Pin, GPIO_PIN_RESET);


	// LED flash to confirm moving to next test
	while(i<3){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(1000);
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
void testLA(){
	int i=0;
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
	HAL_Delay(3000);

	// reverse
	writeLA(0, 1);
	HAL_Delay(3000);

	// forward again
	writeLA(1, 0);
	HAL_Delay(3000);

	// reverse again
	writeLA(0, 1);
	HAL_Delay(3000);


	// there's also the potentiometer that needs to be tested


	// LED flash to confirm moving to next test
	while(i<4){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(1000);
	i++;
	}
}


// test the sensors and their reading
void sensorTest(){
	int i=0;
	int sensor1;
	int sensor2;

	// turn sensors on
	HAL_GPIO_WritePin(SENSOR_ON_GPIO_Port, SENSOR_ON_Pin, GPIO_PIN_SET);

	while(1){
	// checks sensor 1
	if(HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin) == 1){
	sensor1 = 1;
	// if it works, toggle the LED a few times rapidly
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(100);
	}
	// checks 2
	if(HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin) == 1){
	sensor2 = 1;
	// turn on speaker to alert people sensor one is on
	HAL_GPIO_TogglePin(SPKR_GPIO_Port, SPKR_Pin);
	HAL_Delay(1000);
	}

	// only if both sensors work will we exit the test
	if (sensor1 == 1 && sensor2 == 1){
		break;
	}
	}
	//turn off sensors
	HAL_GPIO_WritePin(SENSOR_ON_GPIO_Port, SENSOR_ON_Pin, GPIO_PIN_RESET);

	// LED flash to confirm moving to next test
	while(i<5){
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_Delay(1000);
	i++;
	}
}

// full test routine
void testRoutine(){
	// running through the above test protocols
	ledBlink();
	spkrTest();
	motorTest();
	testLA();
	sensorTest();
}
