
// the purpose of this script is to be able to modify the IO stream
// this is a simulated GPIO system. I will enter in what pin I want to change, 
// and the actual firmware will then read the IO stream to test it out. 
#include <stdio.h>
// first defining the pin numbers 
#define VDD_Pin 1;
#define OSC_IN_Pin 2;
#define OSC_OUT_Pin 3;
#define NRST_Pin 4;
#define VDDA_Pin 5;
#define DCM_SPEED_Pin 6;
#define PA1_Pin 7;
#define USART2_TX_Pin 8;
#define ISO_CHUTE_Pin 9;
#define ISO_DROGUE_Pin 10;
#define DCM_PWM_Pin 11;
#define DCMDIR_Pin 12;
#define DCM_ON_Pin 13;
#define LED_Pin 14;
#define SPKR_Pin 15;
#define VSS_Pin 16;
#define VDDIO2_Pin 17;
#define LA_IN1_Pin 18;
#define LA_IN2_Pin 19;
#define POT_OUT_AN_Pin 20;
#define CAN_RX_Pin 21;
#define CAN_TX_Pin 22;
#define SWDIO_Pin 23;
#define SWCLK 24;
#define USART2_RX_Pin 25;
#define SENSOR_ON_Pin 26;
#define SENSOR2_Pin 27;
#define SENSOR1_Pin 28;
#define BATT_READ_Pin 29;
#define ACOK_Pin 30;
#define PB8_Pin 31;
#define VSSA_Pin 32;


// initiates blank GPIO
void initialize(){
    FILE *fp;
    fp = fopen("gpio_sim.txt", "w");

    for(int i = 0; i<32; i++){
        // initialize all 0
        char input[30];
        sprintf(input, "%d\n", 0);
        fputs(input, fp);
    }

    fclose(fp);
}


// function that actually changes the pin
void change_pin(int pin, int value){
    FILE *fp;
    fp = fopen("gpio_sim.txt", "r");
    int pins[32];
    char buf[20];

    for(int i = 0; i<32; i++){
        fscanf(fp, "%d ", &pins[i]);
    }

    fclose(fp);
    //
    fp = fopen("gpio_sim.txt", "w+");
    for(int i = 0; i<32; i++){
        if (i == pin){
            sprintf(buf, "%d\n", value);
            fputs(buf, fp);
        }
        else
        {
            sprintf(buf, "%d\n", pins[i]);
            fputs(buf , fp);
        }
        
    }
    fclose(fp);
    }

// function that gathers what pin to change
void gather_input(char * buf){
    int pin; 
    int value;
    printf("Enter Pin change -> [Pin] [Value 0/1]: \n");
    scanf("%d %d", &pin, &value);
    printf("Changing Pin %d to %d\n", pin, value);

    change_pin(pin - 1, value);

}

int main(){

    char buf[100];

    initialize();

    while(1){

        gather_input(buf);
        
    }

    return 0;
}
