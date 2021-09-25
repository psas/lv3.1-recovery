// telespoof.ino -- PSAS LV3.1 Recovery
// author: Sean Lai
//
// Spoofs telemetrum signals to deploy main and drogue parachutes
// after receiving a GO signal. For the drop test, GO signal is a
// an opened circuit.
// Usage: drogue_delay --> time from GO to drogue deployment
//        main_delay   --> time from GO to main   deployment
//        Set these to values as defined by flight model.
//        IMPORTANT: The code expects these values in milliseconds
//
// Physical setup:
// -- connect jumper to short (with static line) to armed_pin and GND
// -- connect drogue_pin and main_pin to control board as required

// Setup pin numbers
const int armed_pin = 8;
const int drogue_pin = 9;
const int main_pin = 10;

// debugging pins
// const int drogue_read_pin = 11;
// const int main_read_pin = 12;

void setup() {
    // configure pins
    pinMode(armed_pin, INPUT_PULLUP); // Arming input to ground via a static line: High = armed, Low = safed
    pinMode(drogue_pin, INPUT);       // Yes, it's an output, but low = fire and we'll let it float when not firing
    pinMode(main_pin, INPUT);         // Yes, it's an output, but low = fire and we'll let it float when not firing

    // Serial comms for debugging
    Serial.begin(9600);
}

void loop() {
    // Set these as defined by flight model. Values in milliseconds
    const int drogue_delay = 1500;
    const int main_delay = 6000;

    // Setup state variables
    static int GO_time = 0;
    static int arm_debounce = 0;
    static bool armed = false;
    static bool drogue = false;
    static bool main = false;

    //---------------- Serial debugging ----------------
    // Serial.println(digitalRead(armed_pin));
    // Serial.print("armed = ");
    // Serial.print(armed);
    // Serial.print("  |  drogue deployed = ");
    // Serial.print(drogue);
    // Serial.print("  |  drogue reading = ");
    // Serial.print(digitalRead(drogue_read_pin));
    // Serial.print("  |  main deployed = ");
    // Serial.print(main);
    // Serial.print("  |  main reading = ");
    // Serial.println(digitalRead(main_read_pin));

    // Debounce the arm_pin; it must be unshorted to ground for 100 ms
    // else the count resets. Once we pass 100 ms, we fire the system.
    if (armed == false) {
          if (digitalRead(armed_pin) == 1) {
            ++arm_debounce;
            if (arm_debounce > 9) {
                GO_time = millis();
                armed = true;          
            }
            else {
                arm_debounce = 0;
            }
        }
    }
    else {
        if (millis()-GO_time > drogue_delay) {
            // Send signal to release drogue chute
            pinMode(drogue_pin, OUTPUT);
            digitalWrite(drogue_pin, 0);
            drogue = true;
        }
        if (millis()-GO_time > main_delay) {
            // Send signal to release main chute
            pinMode(main_pin, OUTPUT);
            digitalWrite(main_pin, 0);
            main = true;
        } 
    }

    // Set the time to be >= 10 ms
    delay (10);
}
id|moose>
  
