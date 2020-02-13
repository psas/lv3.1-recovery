
// the purpose of this script is to be able to modify the IO stream
// this is a simulated GPIO system. I will enter in what pin I want to change, 
// and the actual firmware will then read the IO stream to test it out. 
// I decided to switch this over to a python file because C gives me a headache, and this simulator isn't embedded...
#include <stdio.h>
#include <string.h>
// first defining the pin numbers 
#include "pins.h"


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
    char mode[4];
    char input[10];
    // get mode
    printf("__________________\n>Enter Mode: \n");
    gets(mode);
    printf("[INFO] Mode is now: %s\n", mode);
    

    if (strcmp(mode, "pin") == 0){
    
    printf(">Enter Pin change -> [Pin] [Value 0/1]: \n");
    scanf("%d %d", &pin, &value);
    printf("[INFO] Changing Pin %d to %d\n", pin, value);

    change_pin(pin - 1, value);
    }
    if (strcmp(mode, "test") == 0){
        printf(">Enter test procedure: \n");
    }
}

int main(){

    char buf[100];
    // initialize all 0s
    initialize();

    while(1){

        gather_input(buf);
        
    }

    return 0;
}
