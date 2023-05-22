/*
    Fire thread!
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "recovery.h"

//===========================================================================================
// Mainchute!
//===========================================================================================

THD_WORKING_AREA(waMainchuteThread, 256);

THD_FUNCTION(MainchuteThread, arg) {

    (void)arg;
    chRegSetThreadName("mainchute");
    //chprintf(DEBUG_SD, "mainchute thread starting up!\r\n");
  
    static int actuatorPosition = 0;
     
    // Make sure the LA is off
    
    //palClearLine(LINE_LA_DRV1);
    //palClearLine(LINE_LA_DRV2);
    //
    // Set up the ADC
    
    //static ADCConfig adccfg = {};
    
    // Create buffer to store ADC results. This is a one-dimensional interleaved array
    #define ADC_BUF_DEPTH 1 // depth of buffer
    #define ADC_CH_NUM 2    // number of used ADC channels
    static adcsample_t samples[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

    
    /*
    * ADC conversion group.
    * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
    * Channels:    IN1.
    */
    static const ADCConversionGroup adccg = {
    FALSE, // not circular buffer
    2, // just one channel
    NULL, // no call back
    NULL, 
    ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,             /* CFGR1 */ 
    ADC_TR(0, 0),                                     /* TR */
    ADC_SMPR_SMP_1P5,                                 /* SMPR */
    ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL0                              /* CHSELR -- just channel one*/
    };
    
    // Start the ADC
    adcStart(&ADCD1, NULL);

    
    while (true) {

        // We always need to know where we are in the LA
        // Linear Actuator potentiometer is on PA1 = ADC_IN1
        
        palToggleLine(LINE_LED);
        adcConvert (&ADCD1, &adccg, &samples[0], ADC_BUF_DEPTH);

        // UNDERSTANDING THE ACTUATOR POSITION
        // 
        // When the LA is all the way down, the potentiometer is close to 0V
        // When the LA is all the way up, the potentiometer is close to 3.3V
        int hsensor1 = samples[0];
        int hsensor2 = samples[1];
        int sensor1 = (hsensor1 * 3300) /4096;
        int sensor2 = (hsensor2 *3300) /4096 ;
        chprintf(DEBUG_SD, "MainchuteThread: Hall sensor 1 = %d\r\n", sensor1);
        chprintf(DEBUG_SD, "MainchuteThread: Hall sensor 2 = %d\r\n", sensor2);

        // ----------------------------------------------------------------------------------------
        // Main STOP
        // ----------------------------------------------------------------------------------------
        
        if (mainchuteCommand == stop_m) {
           // palClearLine(LINE_LA_DRV1);
           // palClearLine(LINE_LA_DRV2);
            mainchuteCommand = idle_m;       // and we're done here.
            chprintf(DEBUG_SD, "MainchuteThread: Actuator position = %d", actuatorPosition);
       }
       
        // ----------------------------------------------------------------------------------------
        // Firing Main
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LA speed, and check for motor jamming. Try going backwards if necessary.
        
        else if (mainchuteCommand == fire_m) {
            if (actuatorPosition > 310) { // Get the potentiometer down to 0.25 V: 0.25V * 4096 counts/3.3V = 310
                chprintf(DEBUG_SD, "MainchuteThread: FIRING, Position = %d --> 310, MOTOR DOWN\r\n",actuatorPosition);
             //   palClearLine(LINE_LA_DRV1);
             //   palSetLine(LINE_LA_DRV2);
            }
            else {
                chprintf(DEBUG_SD, "MainchuteThread: FIRING, Position = %d --> 310, MOTOR OFF\r\n",actuatorPosition);
                chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
                // OK we're done, we've past the unlock position so we're safely deployed
     //           palClearLine(LINE_LA_DRV1);
     //           palClearLine(LINE_LA_DRV2);
                mainchuteCommand = idle_m;       // and we're done here.
            }
        }

        // ----------------------------------------------------------------------------------------
        // Reseting Main
        // ----------------------------------------------------------------------------------------
        
        // TODO: HANDLE ERROR CASES, LIKE STARTING UP IN MOVING OR WHATEVER
        // TODO: Monitor LA speed, and check for motor jamming. Try going backwards if necessary.

        else if (mainchuteCommand == reset_m) {
            if (actuatorPosition < 3785) { // Get the potentiometer above 3.3-0.25V: (3.3V - 0.25 V) * 4096 counts/3.3V = 3785
                chprintf(DEBUG_SD, "MainchuteThread: RESETING, Position = %d --> 3785, MOTOR UP\r\n",actuatorPosition);
    //            palSetLine(LINE_LA_DRV1);
    //            palClearLine(LINE_LA_DRV2);
            }
            else {
                chprintf(DEBUG_SD, "MainchuteThread: RESETING, Position = %d --> 3785, MOTOR OFF\r\n",actuatorPosition);
                chThdSleepMilliseconds(10); // Wait for printout (100 char ~ 10 ms)
    //            palClearLine(LINE_LA_DRV1);
    //            palClearLine(LINE_LA_DRV2);
                mainchuteCommand = idle_m;       // and we're done here.
            }
        }
        
        // Check every 100 ms
        chThdSleepMilliseconds(1000);//
    }
}