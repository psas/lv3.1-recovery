// telespoof.ino -- PSAS LV3.1 Recovery
// author: Sean Lai
//
// Spoofs telemetrum signals to deploy main and drogue parachutes
// after receiving a GO signal. For the drop test, GO signal is a
// an opened circuit.
// Usage: drogue_delay --> time from GO to drogue deployment
//        main_delay   --> time from GO to main   deployment
//        Set these to values as defined by flight model.
//        IMPORTANT: These values must be converted to milliseconds.
//
// Physical setup:
// -- connect jumper to short (with static line) to jumper_reading_pin and GND
// -- connect drogue_pin and main_pin to control board as required

// Setup pins
const int jumper_reading_pin = 8;
const int drogue_pin = 9;
const int main_pin = 10;

void setup() {
    pinMode(jumper_reading_pin, INPUT_PULLUP);
    pinMode(drogue_pin, OUTPUT);
    pinMode(main_pin, OUTPUT);
}

void loop() {
    // Set these as defined by flight model. Values in milliseconds
    const int drogue_delay = 0;
    const int main_delay = 0;

    static int GO_time = 0;
    static bool armed = false;

    // Check that jumper has shorted and system is not yet armed.
    if (digitalRead(jumper_reading_pin) && armed == false) {
        // Set start time and arm system
        GO_time = millis();
        armed = true;
    }

    if (armed = true){
        if (millis()-GO_time > drogue_delay) {
            // Send signal to release drogue chute
            digitalWrite(drogue_pin, 1);
        }
        if (millis()-GO_time > main_delay) {
            // Send signal to release main chute
            digitalWrite(main_pin, 1);
        } 
    }
}