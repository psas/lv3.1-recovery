#define SWITCH_1 1
#define SWITCH_2 2
#define SWITCH_3 3
#define SWITCH_4 4
#define debug_light 38

long previous = 0;

void setup() {
    Serial1.begin(115200);
    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);
    pinMode(debug_light, OUTPUT);
    pinMode(mode, INPUT);
    pinMode(silence, OUTPUT);
    pinMode(shutdown, OUTPUT);
    pinMode(reciever, OUTPUT);
    pinMode(transmitter, INPUT);

    digitalWrite(debug_light, HIGH);
}

void loop() { // try using println if this doesn't work
    int lock_signal_drogue = digitalRead(SWITCH_1);
    int unlock_signal_drogue = digitalRead(SWITCH_3);
    int lock_signal_main = digitalRead(SWITCH_2);
    int unlock_signal_main = digitalRead(SWITCH_4);

    if (unlock_signal_drogue == 0 || unlock_signal_main == 0) {
        if (millis() - previous >= 100) {
            Serial1.print(u\n);
        }
        previous = millis();
    }

     if (lock_signal_drogue == 0 || lock_signal_main == 0) {
        if (millis() - previous >= 100) {
            Serial1.print(l\n);
        }
        previous = millis();
    }
    delay(50);
}