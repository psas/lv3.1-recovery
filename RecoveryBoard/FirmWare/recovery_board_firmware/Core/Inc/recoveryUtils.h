#ifndef UTILS_H
#define UTILS_H
#include "main.h"

typedef enum {
	SPKR_ON,
	SPKR_OFF
} SPKR_State;

// function to read analog inputs, and return the output
uint16_t analogRead(ADC_HandleTypeDef &hadc);

// function to send text to the serial monitor
void serialSend(uint16_t msg, UART_HandleTypeDef &huart2);

// for reading the motor's pulse
void TIM2_PWM_InputInit();

// function returns the frequency and duty
uint32_t * TIM2_PWM_InputRead();

// turns speaker on or off
void speakerToggle(SPKR_State state, TIM_HandleTypeDef htim);
#endif
