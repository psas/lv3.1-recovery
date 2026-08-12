#define SWITCH_1 1
#define SWITCH_2 2
#define SWITCH_3 3
#define SWITCH_4 4
#define debug_light 38

bool lock_state = 1;
bool unlock_state = 1;
bool last_lock_state = 1;
bool last_unlock_state = 1;
long previous = 0;

void setup() {
    Serial1.begin(115200);
    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);
    pinMode(debug_light, OUTPUT);

    digitalWrite(debug_light, HIGH);
}

void loop() {
    int lock_reading = digitalRead(SWITCH_1);
    int unlock_reading = digitalRead(SWITCH_2);

    if (lock_reading - last_lock_state < 0 || unlock_reading - last_unlock_state < 0) {
        previous = millis();
    }

    if (millis() - previous) > 50) {
        if (lock_reading != lock_state) {
            lock_state = lock_reading;
            if (lock_state == 0) {
                Serial1.print(l\n);
                delay(100);
            }
        }
        if (unlock_reading != unlock_state) {
            unlock_state = unlock_reading;
            if (unlock_state == 0) {
                Serial1.print(u\n);
                delay(100);
            }
        }
    }

    last_lock_state = lock_reading;
    last_unlock_state = unlock_reading;
}