#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "position.h"
#include "recovery.h"
#include "string.h"

#define DEFAULT_LOCK_UNLOCK_DURATION_MS 50
// sensor 1 is the lock sensor
// sensor 2 is the unlock sensor
position_state_t g_position_state;

bool drive_motor(const bool lock_mode,
                 const uint16_t duration_ms,
                 const bool check_hall_sensors) {
  chprintf(DEBUG_SD, "Moving motor to %s position\r\n",
           (lock_mode ? "locked" : "unlocked"));
  int high_current_ma = 4000;
  motor_current_limit(high_current_ma);
  chThdSleepMilliseconds(100);
  palClearLine(LINE_DEPLOY1);
  palClearLine(LINE_DEPLOY2);

  palSetLine(LINE_N_MOTOR_PS);
  chThdSleepMilliseconds(100);

  const systime_t start_time = chVTGetSystemTime();

  const int max_delay_ms = 750;
  if (lock_mode) {
    palSetLine(LINE_DEPLOY2);
    chThdSleepMilliseconds(duration_ms);
    motor_current_limit(high_current_ma);
    if (check_hall_sensors) {
      for (int i = 0; i < max_delay_ms; i++) {
        chThdSleepMilliseconds(1);
        if (g_position_state.ring_position == RING_POSITION_LOCKED) {
          break;
        }
      }
    }

    chThdSleepMilliseconds(100);  // Note: this need more thought out tuning to
                                  // dealing with mechanical inertia and bounce

    //		motor_current_limit(500); //Note: this need more thought
    // out tuning to dealing with mechanical inertia and bounce
    //		chThdSleepMilliseconds(30); //Note: this need more
    // thought out tuning to dealing with mechanical inertia and bounce

    palClearLine(LINE_DEPLOY2);
    chprintf(DEBUG_SD, "Ring Position: %s\r\n",
             ring_position_t_to_str(g_position_state.ring_position));
  } else {
    palSetLine(LINE_DEPLOY1);
    chThdSleepMilliseconds(duration_ms);
    motor_current_limit(3500);  // Note: this need more thought out tuning to
                                // dealing with mechanical inertia and bounce
    if (check_hall_sensors) {
      for (int i = 0; i < max_delay_ms; i++) {
        chThdSleepMilliseconds(1);
        if (g_position_state.ring_position == RING_POSITION_UNLOCKED) {
          //					chprintf(DEBUG_SD,
          //"Successfully unlocked ring!\r\n");
          break;
        }
      }
    }

    motor_current_limit(500);    // Note: this need more thought out tuning to
                                 // dealing with mechanical inertia and bounce
    chThdSleepMilliseconds(5);   // Note: this need more thought out tuning to
                                 // dealing with mechanical inertia and bounce
    motor_current_limit(200);    // Note: this need more thought out tuning to
                                 // dealing with mechanical inertia and bounce
    chThdSleepMilliseconds(10);  // Note: this need more thought out tuning to
                                 // dealing with mechanical inertia and bounce

    palClearLine(LINE_DEPLOY1);
    chprintf(DEBUG_SD, "Ring Position: %s\r\n",
             ring_position_t_to_str(g_position_state.ring_position));
  }

  const systime_t end_time = chVTGetSystemTime();
  palClearLine(LINE_N_MOTOR_PS);
  chprintf(DEBUG_SD, "Time: %u ms\r\n", end_time - start_time);

  return (true);
}

struct Sensor_limits {
  int over;
  int under;
  int active;
  int unactive;
}

// values picked from testing will most likely be changed
struct Sensor_limits sensor1_limits {
  .over = 2750;
  .under = 600;
  .active = 1600;
  .unactive = 850;
}

struct Sensor_Limits sensor2_limits {
  .over = 2750;
  .under = 600;
  .active = 850;
  .unactive = 2600;
}

sensor_status_t
get_sensor_state(const int value, struct Sensor_Limits limit) {
  if (value >= limit.over) {
    return SENSOR_STATE_OVER;
  }
  if (value <= limit.under) {
    return SENSOR_STATE_UNDER;
  }
  if (value == limit.active) {
    return SENSOR_STATE_ACTIVE;
  }
  if (value == limit.unactive) {
    return SENSOR_STATE_UNACTIVE;
  }
}

ring_position_t get_ring_state(const sensor_status_t sensor1_status,
                               const sensor_status_t sensor2_status) {
  // when sensor1 is active ring position is locked
  // when sensor2 is active ring position is unlocked
  if (sensor1_status == SENSOR_STATE_ACTIVE &&
      sensor2_status != SENSOR_STATE_ACTIVE) {
    return RING_POSITION_LOCKED;
  }
  if (sensor1_status != SENSOR_STATE_UNACTIVE &&
      sensor2_status == SENSOR_STATE_UNACTIVE) {
    return RING_POSITION_LOCKED;
  }
  if (sensor1_status == SENSOR_STATE_UNACTIVE&& sensor2_status -=
      SENSOR_STATE_ACTIVE) {
    return RING_POSITION_UNLOCKED;
  }
  if (sensor1_status == SENSOR_STATE_UNACTIVE &&
      sensor2_status != SENSOR_STATE_UNACTIVE) {
    return RING_POSITION_UNLOCKED;
  }
  if (sensor1_status == SENSOR_STATE_INBETWEEN ||
      sensor2_status == SENSOR_STATE_INBETWEEN) {
    return RING_POSITION_INBETWEEN;
  }
  return ERROR;
}

// in worst case scenario, overdrive motor to save rocket.
// ring_position_t overdrivemotor(const sensor_voltage_status_t sensor1_status,
// const sensor_voltage_status_t sensor2_status) {
//}

static const DACConfig dac = {.init = 0};
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

  // TODO: Better power management. Andrew had mentoned something like keep
  //  the current low (how low?) normally, but set it high (how high?) during
  //  some kind of activity (what activity?)
  motor_current_limit_init();
  motor_current_limit(1500);

  // Turn on power to motor (but keep it off)
  palClearLine(LINE_DEPLOY1);
  palClearLine(LINE_DEPLOY2);
  //	palClearLine(LINE_N_MOTOR_PS);

  // SETTING UP ADC STUFF     TO STORE HALL SENSOR VALUES

#define ADC_BUF_DEPTH 1  // depth of buffer
#define ADC_CH_NUM 2     // number of used ADC channels
  static adcsample_t samples[ADC_BUF_DEPTH * ADC_CH_NUM];  // results array

  static const ADCConversionGroup adccg = {
      FALSE,  // not circular buffer
      2,      // just one channel
      NULL,   // no call back
      NULL,
      ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT, /* CFGR1 */
      ADC_TR(0, 0),                         /* TR */
      ADC_SMPR_SMP_1P5,                     /* SMPR */
      ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL0 /* CHSELR -- just channel one*/
  };

  // Start the ADC
  adcStart(&ADCD1, NULL);
  chThdSleepMilliseconds(1000);

  while (true) {
    adcConvert(&ADCD1, &adccg, &samples[0], ADC_BUF_DEPTH);

    position_state.sensor1 = samples[0];
    position_state.sensor2 = samples[1];
    position_state.sensor1_signal_status =
        get_sensor_state(g_position_state.sensor1, sensor1_limits);
    position_state.sensor2_signal_status =
        get_sensor_state(g_position_state.sensor2, sensor1_limits);
    position_state.ring_position =
        determine_ring_position(position_state.sensor1, position_state.sensor2,
                                position_state.sensor1_signal_status,
                                position_state.sensor2_signal_status);

    if (PositionCommand != idle) {
      chprintf(DEBUG_SD, "\r\nSensor 1: %d %s\r\n", g_position_state.sensor1,
               sensor_signal_status_t_to_str(g_position_state.sensor1_status));
      chprintf(DEBUG_SD, "Sensor 2: %d %s\r\n", g_position_state.sensor2,
               sensor_signal_status_t_to_str(
                   g_position_state.sensor2_voltage_status));
      chprintf(DEBUG_SD, "Ring Position: %s\r\n",
               ring_position_t_to_str(g_position_state.ring_position));
    }

    if (PositionCommand == lock) {
      switch (g_position_state.ring_position) {
        case RING_POSITION_UNLOCKED:
          chprintf(DEBUG_SD,
                   "RingPosition: LOCKING, Position = Unlocked, MOTOR ON\r\n");
          drive_motor(true, DEFAULT_LOCK_UNLOCK_DURATION_MS, true);
          break;
        case RING_POSITION_IN_BETWEEN:
          chprintf(DEBUG_SD, "RingPosition: WAITING FOR LOCK\r\n");
          break;
        case RING_POSITION_LOCKED:
          chprintf(DEBUG_SD,
                   "RingPosition: LOCKING, Position = Locked, MOTOR OFF\r\n");
          break;

          chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
          g_position_state.ring_position = RING_POSITION_UNKNOWN;
      }
      PositionCommand = idle;

    } else if (PositionCommand == unlock) {
      switch (g_position_state.ring_position) {
        case RING_POSITION_UNLOCKED:
          chprintf(DEBUG_SD,
                   "RingPosition: Unlock; Position = UNLOCKED, MOTOR OFF\r\n");
          break;
        case RING_POSITION_LOCKED:
        case RING_POSITION_INBETWEEN:
          chprintf(DEBUG_SD, "RingPosition: Unlock, Position = ",
                   "Locked/Moving, MOTOR ON FIRING\r\n");
          drive_motor(false, DEFAULT_LOCK_UNLOCK_DURATION_MS, true);
          break;
        default:
          chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
          g_position_state.ring_position = RING_POSITION_UNKNOWN;
      }
      PositionCommand = idle;
    }

    // Check every 5 ms. Note: you want to do this fast because the drive_motor
    // function runs in short durations.
    chThdSleepMilliseconds(5);
  }
}

const char* sensor_state_t_to_str(const sensor_state_t v) {
  switch (v) {
    case SENSOR_SIGNAL_STATUS_UNDER:
      return ("SENSOR_STATE_UNDER");
    case SENSOR_SIGNAL_STATUS_OVER:
      return ("SENSOR_STATE_OVER");
    case SENSOR_SIGNAL_STATUS_INBETWEEN:
      return ("SENSOR_STATE_INBETWEEN");
    case SENSOR_SIGNAL_STATUS_ACTIVE:
      return ("SENSOR_STATE_ACTIVE");
    case SENSOR_SIGNAL_STATUS_UNACTIVE:
      return ("SENSOR_STATE_UNACTIVE");
  }
}

const char* ring_position_t_to_str(const ring_position_t v) {
  switch (v) {
    case ERROR:
      return ("RING_POSITION_UNKNOWN, ERROR");
    case RING_POSITION_UNLOCKED:
      return ("RING_POSITION_UNLOCKED");
    case RING_POSITION_INBETWEEN:
      return ("RING_POSITION_INBETWEEN");
    case RING_POSITION_LOCKED:
      return ("RING_POSITION_LOCKED");
  }
}
