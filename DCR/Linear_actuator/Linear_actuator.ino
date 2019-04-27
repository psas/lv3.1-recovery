/***********************************************************************************
 *          RobotGeek Linear Actuator Demo   
 *          Incremental Analog Control             
 *  
 *    
 *  The following sketch will allow you to control a Firgelli Linear Actuator using
 *  a RobotGeek Joystick for incremental movement.
 *
 *  Products
 *    http://www.robotgeek.com/robotgeek-pantilt.aspx
 *    http://www.robotgeek.com/robotgeek-geekduino-sensor-kit
 *    http://www.robotgeek.com/robotgeek-joystick
 *    http://www.robotgeek.com/p/power-supply-6vdc-2a.aspx
 *    Firgelli Mini Linear Actuators http://www.robotgeek.com/store/Search.aspx?SearchTerms=firgelli
 *    
 *  Wiring
 *    Linear Actuatoro - Digital Pin 9 
 *    Joystick(Vertica)   - Analog Pin 0
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *  
 *
 *  Control Behavior:
 *    Moving the joysticks will incrementally move the linear actuator. This means that when you
 *    release the joystick to its 'home' position, the actuator will stay in the last place
 *    it was. This allows for incremental position control.
 *
 *  External Resources
 *
 ***********************************************************************************/
//Includes
#include <Servo.h>   //servo library for controlling the linear actuator

//Defines
#define LINEARACTUATORPIN 9         //Linear Actuator Digital Pin

#define JOY_PIN 0              // Joystick Analog Pin

//generic deadband limits - not all joystics will center at 512, so these limits remove 'drift' from joysticks that are off-center.
#define DEADBANDLOW 482   //decrease this value if drift occurs, increase it to increase sensitivity around the center position
#define DEADBANDHIGH 542  //increase this value if drift occurs, decrease it to increase sensitivity around the center position

//max/min puse values in microseconds to send to the servo
#define POS_MIN      1050  //fully retracted
#define POS_MAX      2000 //fully extended

Servo linearActuator;    // create servo object to control the linear actuator
int JSValue;             // variables to hold the last reading from the analog pins for the joystick. The value will be between 0 and 1023
int ValueMapped;        // the joystick values will be changed (or 'mapped') to new values to be sent to the linear actuator.

int LinearValue = 1500;   //current positional value being sent to the linear actuator. Start at the 'centered' position

int speed = 1;        //alter this value to change the speed of the system. Higher values mean higher speeds

void setup() 
{ 
  //initialize servos
  linearActuator.attach(LINEARACTUATORPIN, POS_MIN, POS_MAX);  // attaches/activates the linear actuator on pin LINEARACTUATORPIN 
  
  //use the writeMicroseconds to set the linear actuator to a default centered position
  linearActuator.writeMicroseconds(LinearValue); 

} 

void loop() 
{ 

  /**************Servo Positions *******************************/
  //read the values from the joystick
   JSValue = analogRead(JOY_PIN);
 
   //only update if the joystick is outside the deadzone (i.e. moved oustide the center position)
   if(JSValue > DEADBANDHIGH || JSValue < DEADBANDLOW)
   {
     ValueMapped = map(JSValue, 0, 1023, speed, -speed); //Map analog value from native joystick value (0 to 1023) to incremental change (speed to -speed).
     LinearValue = LinearValue + ValueMapped; //add mapped joystick value to present Value
   }
      
  //use the writeMicroseconds to set the servos to their new positions
  linearActuator.writeMicroseconds(LinearValue); 
  
  delay(10); // waits for the servo to get to they're position before continuing 
} 
