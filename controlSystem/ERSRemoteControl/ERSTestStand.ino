#define SWITCH_1 1
#define SWITCH_2 2
#define SWITCH_3 3
#define SWITCH_4 4
#define debug_light 38
#define mode 5
#define silence 6
#define shutdown 7
#define reciever 8
#define transmitter 9

void setup() {
    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);
    pinMode(debug_light, OUTPUT);
    pinMode(mode, INPUT);
    pinMode(silence, OUTPUT);
    pinMode(shutdown, OUTPUT);
    pinMode(reciever, OUTPUT);
    pinMode(transmitter, OUTPUT);
}

void loop() {
    
}