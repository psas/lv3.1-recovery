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
// -- connect jumper to short (with static line) to jumper_reading_pin and GND
// -- connect drogue_pin and main_pin to control board as required

// Setup pin numbers
const int jumper_reading_pin = 8;
const int drogue_pin = 9;
const int main_pin = 10;

// debugging pins
// const int drogue_read_pin = 11;
// const int main_read_pin = 12;

void setup() {
    // configure pins
    pinMode(jumper_reading_pin, INPUT_PULLUP);
    pinMode(drogue_pin, OUTPUT);
    pinMode(main_pin, OUTPUT);

    // pinMode(drogue_read_pin, INPUT);
    // pinMode(main_read_pin, INPUT);

    // Serial comms for debugging
    Serial.begin(9600);
}

void loop() {
    // Set these as defined by flight model. Values in milliseconds
    const int drogue_delay = 1500;
    const int main_delay = 6000;

    // Setup state variables
    static int GO_time = 0;
    static bool armed = false;
    static bool drogue = false;
    static bool main = false;

    //---------------- Serial debugging ----------------
    // Serial.println(digitalRead(jumper_reading_pin));
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

    // Check that jumper has shorted and system is not yet armed.
    if (digitalRead(jumper_reading_pin) == 1 && armed == false) {
        // Set GO_time and arm system
        GO_time = millis();
        armed = true;
    }

    if (armed == true){
        if (millis()-GO_time > drogue_delay) {
            // Send signal to release drogue chute
            digitalWrite(drogue_pin, 1);
            drogue = true;
        }
        if (millis()-GO_time > main_delay) {
            // Send signal to release main chute
            digitalWrite(main_pin, 1);
            main = true;
        } 
    }
}
