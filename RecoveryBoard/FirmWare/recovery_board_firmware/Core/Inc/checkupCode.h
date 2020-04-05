/*
 * checkupCode.h
 *
 *  Created on: Mar 10, 2020
 *      Author: benko
 */

#ifndef SRC_CHECKUPCODE_H_
#define SRC_CHECKUPCODE_H_
#include "main.h"

// test the led by blinking twice a second
void ledBlink();

// test the speaker
void spkrTest();

// test the motor
void motorTest();

// function to make running the H Bridge easier
void writeLA(int IN1, int IN2);

//test the linear actuator
void testLA(ADC_HandleTypeDef &hadc);

// test one sensor
void sensorTestSingle(int sensor);

// test the sensors and their reading
void sensorTestDouble();

// full test routine
void testRoutine();

// reading the telemetrum signals
void telemetrumTest();

#endif /* SRC_CHECKUPCODE_H_ */
