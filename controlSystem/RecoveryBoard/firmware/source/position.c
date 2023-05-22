/*
    Fire thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

// State      S1 S2 Case
// Locked     1  0   2
// Inbetween  1  1   3
// Unlocked   0  1   1
// Spinning   0  0   0

#define LOCKED    2        
//#define RING_MOVING          
#define UNLOCKED  1   
#define INBETWEEN   3
//#define RING_SPINNING  0      



//===========================================================================================
// Position!
//===========================================================================================

THD_WORKING_AREA(waPositionThread, 256);

THD_FUNCTION(PositionThread, arg) {

    (void)arg;
    chRegSetThreadName("Position");
    //chprintf(DEBUG_SD, "Position thread starting up!\r\n");
  
    static int ringPosition = 0;
    
    
    // Turn on the power to the sensors TODO: power them on only when we need them.
   // palSetLine(LINE_ROTSENSE_PWR);
    
    // Turn on power to motor (but keep it off) TODO: Better power management
    palClearLine(LINE_DEPLOY1);
    palSetLine(LINE_DEPLOY2);
    chThdSleepMilliseconds(1000); // Wait for lines to settle
    palSetLine(LINE_DEPLOY1);

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

        // We always need to know where we are in the LA
        // Linear Actuator potentiometer is on PA1 = ADC_IN1
        
        palToggleLine(LINE_LED);
        adcConvert (&ADCD1, &adccg, &samples[0], ADC_BUF_DEPTH);

        int hsensor1 = samples[0];
        int hsensor2 = samples[1];
        int sensor1 = (hsensor1 * 3300) /4096;
        int sensor2 = (hsensor2 *3300) /4096 ;
        chprintf(DEBUG_SD, "MainchuteThread: Hall sensor 1 = %d\r\n", sensor1);
        chprintf(DEBUG_SD, "MainchuteThread: Hall sensor 2 = %d\r\n", sensor2);
 
    

        
        // We always need to know where we are in the ring
        //sensor1 = palReadLine(LINE_ROTSENSE1); //read value of hall sensor 1
        //sensor2 = palReadLine(LINE_ROTSENSE2); // sensor 2

        // ----------------------------------------------------------------------------------------
        // Position STOP
        // ----------------------------------------------------------------------------------------
        
        
            
            
            chprintf(DEBUG_SD, "RingPosition: Ring position = ");

        //PRINTING RINGPOSITION STATUS

            if (sensor2 <= 500) {          //UNDERVOLTAGE
                if (sensor1<500){
                    chprintf(DEBUG_SD,"UNDERVOLTAGE");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                } 
                if(sensor1>=500 && sensor1<1000){
                    chprintf(DEBUG_SD,"UNLOCKED ONE SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                   
                }
                if(sensor1>=1000 && sensor1<2250){
                    chprintf(DEBUG_SD,"INBETWEEN ONE SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=2250 && sensor1<2750){
                    chprintf(DEBUG_SD,"LOCKED");
                    ringPosition = LOCKED;
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
                if(sensor1>=2750){
                    chprintf(DEBUG_SD,"OVERVOLTAGE");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
            }

            else if (sensor2 >= 500 && sensor2<1000) {      //UNLOCK
                if (sensor1<500){
                    chprintf(DEBUG_SD,"UNLOCKED");
                    ringPosition = UNLOCKED;
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                } 
                if(sensor1>=500 && sensor1<1000){
                    chprintf(DEBUG_SD,"UNLOCKED");
                    ringPosition = UNLOCKED;
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
                if(sensor1>=1000 && sensor1<2250){
                    chprintf(DEBUG_SD,"UNLOCKED_INBETWEEN");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
                if(sensor1>=2250 && sensor1<2750){
                    chprintf(DEBUG_SD,"CONFLICT");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)   
                }
                if(sensor1>=2750){
                    chprintf(DEBUG_SD,"UNLOCKED_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)   
                }
                
            }

            else if (sensor2 >= 1000 && sensor2<2250) {     //IN BW
                //chprintf(DEBUG_SD,"IT WORKS??");
                if (sensor1<500){
                    chprintf(DEBUG_SD,"INBETWEEN_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                } 
                if(sensor1>=500 && sensor1<1000){
                    chprintf(DEBUG_SD,"UNLOCKED_INBETWEEN");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=1000 && sensor1<2250){
                    chprintf(DEBUG_SD,"INBETWEEN");
                    ringPosition = INBETWEEN;
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
                if(sensor1>=2250 && sensor1<2750){
                    chprintf(DEBUG_SD,"LOCKED_INBETWEEN");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=2750){
                    chprintf(DEBUG_SD,"INBETWEEN_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
            }

            else if (sensor2 >= 2250 && sensor2<2750) {     //LOCKED
                if (sensor1<500){
                    chprintf(DEBUG_SD,"LOCKED_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                } 
                if(sensor1>=500 && sensor1<1000){
                    chprintf(DEBUG_SD,"CONFLICT");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=1000 && sensor1<2250){
                    chprintf(DEBUG_SD,"LOCKED_INBETWEEN");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=2250 && sensor1<2750){
                    chprintf(DEBUG_SD,"LOCKED");
                    ringPosition = LOCKED;
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                }
                if(sensor1>=2750){
                    chprintf(DEBUG_SD, "LOCKED_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
            }

            else if (sensor2<2750) {     //OVERVOLTAGE
                if (sensor1<500){
                    chprintf(DEBUG_SD, "OVERUNDERVOLTAGE");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                } 
                if(sensor1>=500 && sensor1<1000){
                    chprintf(DEBUG_SD, "UNLOCKED_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=1000 && sensor1<2250){
                    chprintf(DEBUG_SD, "INBETWEEN_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=2250 && sensor1<2750){
                    chprintf(DEBUG_SD, "LOCKED_ONE_SENSOR");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                if(sensor1>=2750){
                    chprintf(DEBUG_SD,"OVERVOLTAGE");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    
                }
                
            }
            chprintf(DEBUG_SD, "\r\n");
            
            switch (ringPosition) {
                 case UNLOCKED:
                    chprintf(DEBUG_SD, "UNLOCKED (10)\r\n");
                    break;
        
                case INBETWEEN:
                    chprintf(DEBUG_SD, "INBETWEEN (11)\r\n");
                    break;
            
                case LOCKED:
                    chprintf(DEBUG_SD, "LOCKED (01)\r\n");
                    break;

               // case RING_SPINNING:
                 //   chprintf(DEBUG_SD, "SPINNING (00)\r\n");
                   // break;

                default:
                    chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
       

        // ----------------------------------------------------------------------------------------
        // Firing Position
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.
        
        

        // ----------------------------------------------------------------------------------------
        // Locking Position
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.

        if (PositionCommand== lock) {
            switch (ringPosition) {    
                case UNLOCKED:
                    //Command is lock and we're unlocked, so run the motor
                    chprintf(DEBUG_SD, "RingPosition: LOCKING, Position = Unlocked, MOTOR ON\r\n");
                    chThdSleepMilliseconds(100); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DEPLOY1); // turn off
                    palClearLine(LINE_DEPLOY2); // turn off
                    palSetLine(LINE_DEPLOY2); // PB5 high
                    
                    break;
                case INBETWEEN:
                    //Command is lock and we're inbetween, so wait for lock
                    chprintf(DEBUG_SD,"WAITING FOR LOCK");
                    break;
            

                case LOCKED:
                    //Command is lock, but we're already locked. So stop motors and exit
                    chprintf(DEBUG_SD, "RingPosition: LOCKING, Position = Locked, MOTOR OFF\r\n");
                    chThdSleepMilliseconds(100); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DEPLOY1); // turn off motor
                    palClearLine(LINE_DEPLOY2); // turn off motor
                    
                    PositionCommand = idle;
                    break;

                default:
                    chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
        }

        // ----------------------------------------------------------------------------------------
        // Unlock Position -- go to unlock state form either side
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LINE_DCM_SPEED, and check for motor jamming. Try going backwards if necessary.

        else if (PositionCommand == unlock) {
            switch (ringPosition) {
                case UNLOCKED:
                    chprintf(DEBUG_SD, "RingPosition: Unlock; Position = UNLOCKED, MOTOR OFF\r\n");
                    chThdSleepMilliseconds(100); // Wait          for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DEPLOY1); // clear
                    palClearLine(LINE_DEPLOY2); // clear
                    PositionCommand = idle;
                    break;

                case LOCKED:
                case INBETWEEN:
                    chprintf(DEBUG_SD, "RingPosition: Unlock, Position = Locked/Moving, MOTOR ON FIRING\r\n");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    palClearLine(LINE_DEPLOY1); // clear
                    palClearLine(LINE_DEPLOY2); // clear
                    palSetLine(LINE_DEPLOY1); // Full blast on
                    break;
        
                default:
                    chprintf(DEBUG_SD, "RingPosition: INVALID POSITION.\r\n");
                    chThdSleepMilliseconds(1000); // Wait for printout (100 char ~ 10 ms)
                    ringPosition = 0;
                }
        }

                    
        // Check every 100 ms
        chThdSleepMilliseconds(100);        
    }}
 

