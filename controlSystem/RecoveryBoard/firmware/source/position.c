#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"
#include "position.h"
#include "string.h"

#define DEFAULT_LOCK_UNLOCK_DURATION_MS      50

position_state_t g_position_state;

bool drive_motor(const bool lock_mode, const uint16_t duration_ms, const bool check_hall_sensors) {
	chprintf(DEBUG_SD, "Moving motor to %s position\r\n", (lock_mode ? "locked" : "unlocked"));
	int high_current_ma = 4000;
	motor_current_limit(high_current_ma);
	chThdSleepMilliseconds(100);
	palClearLine(LINE_DEPLOY1);
	palClearLine(LINE_DEPLOY2);

	palSetLine(LINE_N_MOTOR_PS);
	chThdSleepMilliseconds(100);

	const systime_t start_time = chVTGetSystemTime();

	const int max_delay_ms = 750;
	if( lock_mode ) {
		palSetLine(LINE_DEPLOY2);
		chThdSleepMilliseconds(duration_ms);
		motor_current_limit(high_current_ma);
		if( check_hall_sensors ) {
			for(int i = 0; i < max_delay_ms; i++ ) {
				chThdSleepMilliseconds(1);
				if( g_position_state.ring_position == RING_POSITION_LOCKED ) {
					break;
				}
			}
		}

		chThdSleepMilliseconds(100); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce

//		motor_current_limit(500); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce
//		chThdSleepMilliseconds(30); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce

		palClearLine(LINE_DEPLOY2);
		chprintf(DEBUG_SD, "Ring Position: %s\r\n", ring_position_t_to_str(g_position_state.ring_position));
	} else {
		palSetLine(LINE_DEPLOY1);
		chThdSleepMilliseconds(duration_ms);
		motor_current_limit(3500); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce
		if( check_hall_sensors ) {
			for(int i = 0; i < max_delay_ms; i++ ) {
				chThdSleepMilliseconds(1);
				if( g_position_state.ring_position == RING_POSITION_UNLOCKED ) {
//					chprintf(DEBUG_SD, "Successfully unlocked ring!\r\n");
					break;
				}
			}
		}

		motor_current_limit(500); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce
		chThdSleepMilliseconds(5); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce
		motor_current_limit(200); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce
		chThdSleepMilliseconds(10); //Note: this need more thought out tuning to dealing with mechanical inertia and bounce

		palClearLine(LINE_DEPLOY1);
		chprintf(DEBUG_SD, "Ring Position: %s\r\n", ring_position_t_to_str(g_position_state.ring_position));
	}

	const systime_t end_time = chVTGetSystemTime();
	palClearLine(LINE_N_MOTOR_PS);
	chprintf(DEBUG_SD, "Time: %u ms\r\n", end_time - start_time);

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
//	const int sensor_1_lock_threshold = 2400;
//	const int sensor_2_lock_threshold = 1800;

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

static const DACConfig dac = { .init=0 };
void motor_current_limit_init(void) {
	dacStart(&DACD1, &dac);
}

void motor_current_limit(const uint32_t mA) {
	// MAX 2.75A or 2750mA.VREF must be < 2v, given MOTOR_ILIM is up to
	// 3.3v and gets divided by 2, it can put at most 1.65v on VREF.
	// See below for conversion to mA.
	chDbgAssert(mA <= 2750, "motor_current_limit too high");
	// Current is controlled by setting a voltage on VREF of the BD63150AFM
	// motor driver. The current is determined by the formula VREF/3/RNF
	// where RNF is a current sensing resister chosen by us to be 0.1Ohm.
	// We can set VREF by the DAC on the MOTOR_ILIM pin, PA4, except it
	// passes through a 1/2 voltage divider. So for us:
	//    Iout = MOTOR_ILIM * 5/3
	// The DAC stepsize is 3.3v/2^12 so:
	//    Iout = step * 11/8192
	// Solving for step and using mA instead of A gives:
	dacPutChannelX(&DACD1, 0, (mA * 1024) / 1375);
}

//===========================================================================================
// Position!
//===========================================================================================

THD_WORKING_AREA(waPositionThread, 256);

THD_FUNCTION(PositionThread, arg) {
    (void)arg;
    chRegSetThreadName("Position");
    memset(&g_position_state, 0, sizeof(g_position_state));

    //TODO: Better power management. Andrew had mentoned something like keep
    // the current low (how low?) normally, but set it high (how high?) during
    // some kind of activity (what activity?)
    motor_current_limit_init();
    motor_current_limit(1500);

    // Turn on power to motor (but keep it off)
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
    
    while (true) {
        adcConvert (&ADCD1, &adccg, &samples[0], ADC_BUF_DEPTH);

        const int hsensor1 = samples[0];
        const int hsensor2 = 4096 - samples[1];//Invert the reading for sensor 2

        g_position_state.sensor1 = (hsensor1 * 3300) /4096;
        g_position_state.sensor2 = (hsensor2 * 3300) /4096 ;
    
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
                    drive_motor(true, DEFAULT_LOCK_UNLOCK_DURATION_MS, true);
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
                    drive_motor(false, DEFAULT_LOCK_UNLOCK_DURATION_MS, true);
					break;
				default:
					chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
					g_position_state.ring_position = RING_POSITION_UNKNOWN;
			}
			PositionCommand = idle;
        }

                    
        // Check every 5 ms. Note: you want to do this fast as the drive_motor function runs in short durations.
        chThdSleepMilliseconds(5);
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
