#include "recoveryUtils.h"
#include "main.h"
#include <cstring>
#include <cstdio>

// function to read analog inputs, and return the output
uint16_t analogRead(ADC_HandleTypeDef &hadc){
	uint16_t raw;
	//start the conversion of ADC value
	HAL_ADC_Start(&hadc);
	// stall the processor until the ADC is done
	HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
	raw = HAL_ADC_GetValue(&hadc);

	return raw;
}

// function to send text to the serial monitor
void serialSend(char * msg, UART_HandleTypeDef &huart2){
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
}

// function to set up and read the PWM signal on the proper GPIO
void TIM2_PWM_InputInit(){
	// GPIOA clock already started
	// set pin PA0 in alternate function mode
	GPIOA->MODER |= GPIO_MODER_MODER0_1;

	// set pin PA0 to AF2 form TIM2 CH1
	GPIOA->AFR[0] |= (0x2UL << GPIO_AFRL_AFRL0_Pos);

	__HAL_RCC_TIM2_CLK_ENABLE();
	//setting TIM2 for input capture
	// with channel 1 mapping to TI1 (CC1S = 01)
	// and channel 2 mapped to TI1 (CC2S = 10)
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;

	// setting slave mode (SMS = 100)
	// trigger input (TS = 101)
	TIM2->SMCR |= TIM_SMCR_TS_0 | TIM_SMCR_TS_2 |TIM_SMCR_SMS_2;

	// setting rising edge polarity for channel 1 (CC1P = 0, reset state and CC1NP = 0, reset value)
	// setting falling edge polarity for channel 2 (CC2P = 1 and CC2NP = 0, reset value)
	// enabling capture mode in channels 1 and 2 (CC1E = 1) and (CC2E = 1)
	TIM2->CCER |= TIM_CCER_CC2P | TIM_CCER_CC1E | TIM_CCER_CC2E;

	TIM2->CR1 |= TIM_CR1_CEN;

}

// function returns the frequency and duty
uint32_t * TIM2_PWM_InputRead(){
	static uint32_t result[3];
	uint32_t invDuty;
	uint32_t period;
	uint32_t frequency;
	uint32_t pulse;

	period = TIM2->CCR1;
	invDuty = period / TIM2->CCR2;
	pulse = TIM2->CCR2 - period;
	frequency = 8000000 / period;

	result[0] = frequency;
	result[1] = pulse;
	result[2] = invDuty;

	return result;
}

void speakerToggle(SPKR_State state, TIM_HandleTypeDef htim){
	// start the PWM channel for the speaker
	// 4 KHz, 50% Duty
	if (state == SPKR_ON){
		HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1);
		TIM3->CCR1 = 499;
	}
	else{
		HAL_TIM_PWM_Stop(&htim, TIM_CHANNEL_1);
	}
}
