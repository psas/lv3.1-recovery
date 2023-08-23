#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"
#include "position.h"
#include "string.h"

#define DEFAULT_LOCK_UNLOCK_DURATION_MS      50


#define PWM_TIMER_FREQ	1000000 // Hz
#define PWM_FREQ		20000// periods per sec
#define PWM_PERIOD		PWM_TIMER_FREQ/PWM_FREQ

//PB4, DEPLOY_1, is TIM3_CH1
//PB5, DEPLOY_2 is TIM3_CH2
static PWMConfig pwmcfg_3 = {
  .frequency = PWM_TIMER_FREQ,
  .period = PWM_PERIOD,
  .callback = NULL,
  .channels = {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  .cr2 = 0,//CR2
 #if STM32_PWM_USE_ADVANCED
   .bdtr = 0, //BDTR
 #endif
   .dier = 0,//DIER
};



//PB15 is TIM1_CH3N complimentary output
//static PWMConfig pwmcfg_1 = {
//  .frequency = PWM_TIMER_FREQ,
//  .period = PWM_PERIOD,
//  .callback = NULL,
//  .channels = {
//   {PWM_OUTPUT_ACTIVE_LOW | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, NULL},
//   {PWM_OUTPUT_ACTIVE_LOW | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, NULL},
//   {PWM_OUTPUT_ACTIVE_LOW | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, NULL},
//   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
//  },
//  .cr2 = TIM_CR2_MMS_1,//CR2
// #if STM32_PWM_USE_ADVANCED
//   .bdtr = 0, //BDTR
// #endif
//   .dier = 0,//DIER
//};


static const DACConfig dac_config = {
  .init         = 2047u,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

position_state_t g_position_state;





//void set_motor_pwm_output(uint32_t pwm_percent, uint32_t channel_number) {
//	if( PWMD1.state == PWM_STOP ) {
//		pwmStart(&PWMD3, &pwmcfg_3);
//#if 0
//		chprintf(DEBUG_SD, "Turning on PWM output...\r\n");
//		pwmEnableChannel(&PWMD1, MT_X_PWM_PWM_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000));
//		pwmEnableChannel(&PWMD1, MT_Y_PWM_PWM_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000));
//		pwmEnableChannel(&PWMD1, MT_Z_PWM_PWM_CHANNEL, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000));
//#endif
//	}
//
//
//	pwmDisableChannel(&PWMD3, channel_number);
//	pwmEnableChannel(&PWMD3, channel_number, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, pwm_percent));
//
//}


bool drive_motor(const bool lock_mode, const uint16_t duration_ms) {
	chprintf(DEBUG_SD, "Moving motor to %s position\r\n", (lock_mode ? "locked" : "unlocked"));
	chThdSleepMilliseconds(100);
	palClearLine(LINE_DEPLOY1); // clear
	palClearLine(LINE_DEPLOY2); // clear

	palSetLine(LINE_N_MOTOR_PS);
	chThdSleepMilliseconds(100);

	if( lock_mode ) {
		palSetLine(LINE_DEPLOY1); // Full blast on
		chThdSleepMilliseconds(duration_ms);
		palClearLine(LINE_DEPLOY1); // clear
	} else {
		palSetLine(LINE_DEPLOY2); // Full blast on
		chThdSleepMilliseconds(duration_ms);
		palClearLine(LINE_DEPLOY2); // clear
	}

	palClearLine(LINE_N_MOTOR_PS);

	return(true);
}

sensor_voltage_status_t get_sensor_voltage_status(const int reading_value, const hall_sensor_t which_sensor) {
	const int low_threshold = (which_sensor == HALL_SENSOR_1 ? 500 : 500);//FIXME handle different cutoff thresholds for the different sensor referencing which_sensor
	const int high_threshold = (which_sensor == HALL_SENSOR_1 ? 2750 : 2750);//FIXME handle different cutoff thresholds for the different sensor referencing which_sensor

	if( reading_value <= low_threshold ) {
		return(SENSOR_VOLTAGE_STATUS_UNDER_VOLTAGE);
	}
	if( reading_value > high_threshold ) {
		return(SENSOR_VOLTAGE_STATUS_OVER_VOLTAGE);
	}

	return(SENSOR_VOLTAGE_STATUS_OK);
}

ring_position_t determine_ring_position(const int sensor1, const int sensor2, const sensor_voltage_status_t sensor1_status, const sensor_voltage_status_t sensor2_status) {
	const int sensor_1_unlock_threshold = 1000;
	const int sensor_2_unlock_threshold = 1000;
	const int sensor_1_lock_threshold = 2250;
	const int sensor_2_lock_threshold = 2250;

	if( sensor1_status == SENSOR_VOLTAGE_STATUS_OK && sensor2_status == SENSOR_VOLTAGE_STATUS_OK ) {
		if( sensor1 < sensor_1_unlock_threshold ) {
			return(RING_POSITION_LOCKED);
		} else if( sensor1 < sensor_1_lock_threshold ) {
			return(RING_POSITION_IN_BETWEEN);
		}
		return(RING_POSITION_UNLOCKED);

	} else if( sensor1_status == SENSOR_VOLTAGE_STATUS_OK ) {
		if( sensor1 < sensor_1_unlock_threshold ) {
			return(RING_POSITION_LOCKED);
		} else 	if( sensor1 < sensor_1_lock_threshold ) {
			return(RING_POSITION_IN_BETWEEN);
		}
		return(RING_POSITION_UNLOCKED);
	} else if( sensor2_status == SENSOR_VOLTAGE_STATUS_OK ) {
		if( sensor2 < sensor_2_unlock_threshold ) {
			return(RING_POSITION_LOCKED);
		} else if( sensor2 < sensor_2_lock_threshold ) {
			return(RING_POSITION_IN_BETWEEN);
		}
		return(RING_POSITION_UNLOCKED);
	}

	return(RING_POSITION_UNKNOWN);
}


//===========================================================================================
// Position!
//===========================================================================================

THD_WORKING_AREA(waPositionThread, 512);

THD_FUNCTION(PositionThread, arg) {
    (void)arg;
    chRegSetThreadName("Position");
    memset(&g_position_state, 0, sizeof(g_position_state));
    
    // Turn on power to motor (but keep it off) TODO: Better power management
    palClearLine(LINE_DEPLOY1);
    palClearLine(LINE_DEPLOY2);
//	palClearLine(LINE_N_MOTOR_PS);

//SETTING UP ADC STUFF     TO STORE HALL SENSOR VALUES

    #define ADC_BUF_DEPTH 1 // depth of buffer
    #define ADC_CH_NUM 2   // number of used ADC channels
    static adcsample_t samples[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

	static const ADCConversionGroup adccg = {
		FALSE, // not circular buffer
		2, // just one channel
		NULL, // no call back
		NULL,
		ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,             /* CFGR1 */
		ADC_TR(0, 0),                                     /* TR */
		ADC_SMPR_SMP_1P5,                                 /* SMPR */
		ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL0 /* CHSELR -- just channel one*/
    };
    
    // Start the ADC
    adcStart(&ADCD1, NULL);
    chThdSleepMilliseconds(1000);
    


    pwmStart(&PWMD3, &pwmcfg_3);
    pwmDisableChannel(&PWMD3, 0);
    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, 5000));




	palSetPadMode(GPIOA, GPIOA_MOTOR_ILIM, PAL_MODE_INPUT_ANALOG);
    dacStart(&DACD1, &dac_config);
    dacPutChannelX(&DACD1, 0, 3600); //3V
//    dacPutChannelX(&DACD1, 0, 2400); //2V
//    dacPutChannelX(&DACD1, 0, 1800); //1.5V
//      dacPutChannelX(&DACD1, 0, 1440); //1.2V
//    dacPutChannelX(&DACD1, 0, 1200); //1V
//    dacPutChannelX(&DACD1, 0, 600); //0.5V
//    dacPutChannelX(&DACD1, 0, 480); //0.4V
//    dacPutChannelX(&DACD1, 0, 360); //0.3V
//    dacPutChannelX(&DACD1, 0, 240); //0.2V
//    dacPutChannelX(&DACD1, 0, 120); //0.1V

	palSetLine(LINE_N_MOTOR_PS);
//    palSetLine(LINE_DEPLOY1); // Full blast on
	palClearLine(LINE_DEPLOY2); // clear
	while (true) {
		chprintf(DEBUG_SD, "Sleeping...\r\n");
		for(uint32_t per = 0; per < 5000; per += 100 ) {
		    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, per));
	    	chThdSleepMilliseconds(500);
		}

//		for(uint32_t v = 2400; v <= 3600; v += 120 ) {
//			chprintf(DEBUG_SD, "Setting output to %u mV\r\n", (v * 100 / 120));
//	    	dacPutChannelX(&DACD1, 0, v);
//	    	chThdSleepMilliseconds(5000);
//		}

    	chThdSleepMilliseconds(3000);
	}





    while (true) {
        adcConvert (&ADCD1, &adccg, &samples[0], ADC_BUF_DEPTH);

        const int hsensor1 = samples[0];
        const int hsensor2 = 4096 - samples[1];//Invert the reading for sensor 2

        g_position_state.sensor1 = (hsensor1 * 3300) /4096;
        g_position_state.sensor2 = (hsensor2 *3300) /4096 ;
    
        g_position_state.sensor1_voltage_status = get_sensor_voltage_status(g_position_state.sensor1, HALL_SENSOR_1);
        g_position_state.sensor2_voltage_status = get_sensor_voltage_status(g_position_state.sensor2, HALL_SENSOR_2);
        g_position_state.ring_position = determine_ring_position(g_position_state.sensor1, g_position_state.sensor2, g_position_state.sensor1_voltage_status, g_position_state.sensor2_voltage_status);

        if (PositionCommand != idle ) {
			chprintf(DEBUG_SD, "\r\nSensor 1: %d %s\r\n", g_position_state.sensor1, sensor_voltage_status_t_to_str(g_position_state.sensor1_voltage_status));
			chprintf(DEBUG_SD, "Sensor 2: %d %s\r\n", g_position_state.sensor2, sensor_voltage_status_t_to_str(g_position_state.sensor2_voltage_status));
			chprintf(DEBUG_SD, "Ring Position: %s\r\n", ring_position_t_to_str(g_position_state.ring_position));
        }

        
        if (PositionCommand== lock) {
            switch (g_position_state.ring_position) {
                case RING_POSITION_UNLOCKED:
                    //Command is lock and we're unlocked, so run the motor
                    chprintf(DEBUG_SD, "RingPosition: LOCKING, Position = Unlocked, MOTOR ON\r\n");
                    drive_motor(true, DEFAULT_LOCK_UNLOCK_DURATION_MS);
                    break;
                case RING_POSITION_IN_BETWEEN:
                    //Command is lock and we're in between, so wait for lock
                    chprintf(DEBUG_SD, "RingPosition: WAITING FOR LOCK\r\n");
                    break;
                case RING_POSITION_LOCKED:
                    //Command is lock, but we're already locked. So stop motors and exit
                    chprintf(DEBUG_SD, "RingPosition: LOCKING, Position = Locked, MOTOR OFF\r\n");
                    break;

                default:
                    chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
                    g_position_state.ring_position = RING_POSITION_UNKNOWN;
            }
			PositionCommand = idle;

        } else if (PositionCommand == unlock) {
			// ----------------------------------------------------------------------------------------
			// Unlock Position -- go to unlock state form either side
			// ----------------------------------------------------------------------------------------
			switch (g_position_state.ring_position) {
				case RING_POSITION_UNLOCKED:
					chprintf(DEBUG_SD, "RingPosition: Unlock; Position = UNLOCKED, MOTOR OFF\r\n");
					break;
				case RING_POSITION_LOCKED:
				case RING_POSITION_IN_BETWEEN:
					chprintf(DEBUG_SD, "RingPosition: Unlock, Position = Locked/Moving, MOTOR ON FIRING\r\n");
                    drive_motor(false, DEFAULT_LOCK_UNLOCK_DURATION_MS);
					break;
				default:
					chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
					g_position_state.ring_position = RING_POSITION_UNKNOWN;
			}
			PositionCommand = idle;
        }

                    
        // Check every 100 ms
        chThdSleepMilliseconds(100);
    }
}

const char* sensor_voltage_status_t_to_str(const sensor_voltage_status_t v) {
	switch (v) {
	case SENSOR_VOLTAGE_STATUS_UNKNOWN:
		return ("SENSOR_VOLTAGE_STATUS_UNKNOWN");
	case SENSOR_VOLTAGE_STATUS_OK:
		return ("SENSOR_VOLTAGE_STATUS_OK");
	case SENSOR_VOLTAGE_STATUS_UNDER_VOLTAGE:
		return ("SENSOR_VOLTAGE_STATUS_UNDER_VOLTAGE");
	case SENSOR_VOLTAGE_STATUS_OVER_VOLTAGE:
		return ("SENSOR_VOLTAGE_STATUS_OVER_VOLTAGE");
	}
	return ("???");
}

const char* ring_position_t_to_str(const ring_position_t v) {
	switch (v) {
	case RING_POSITION_UNKNOWN:
		return ("RING_POSITION_UNKNOWN");
	case RING_POSITION_UNLOCKED:
		return ("RING_POSITION_UNLOCKED");
	case RING_POSITION_IN_BETWEEN:
		return ("RING_POSITION_IN_BETWEEN");
	case RING_POSITION_LOCKED:
		return ("RING_POSITION_LOCKED");
	}
	return("???");
}
