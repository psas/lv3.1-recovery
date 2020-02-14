#include<iostream>
#include<fstream>
#include<string>
#include<windows.h>
#include"pins.h"

using namespace std;


// checks all gpio pins in the simulation
int * gpio_check(){
    static int pins[32];
    string line;

    ifstream datafile;
    datafile.open("gpio_sim.txt");

    int count = 0;
    while(count<32){
    datafile >> pins[count];
    // cout << pins[count];
    count++;
    }
    // cout << "\n";
    
    datafile.close();
    return pins;
}


// function to check what state the sensors are in
int sensorState(){
    int * pinstatus;
    int sensor1;
    int sensor2;
    int state;

    pinstatus = gpio_check();
    // subtracting 1 to match the proper array index
    sensor1 = pinstatus[SENSOR1_Pin - 1];
    sensor2 = pinstatus[SENSOR2_Pin - 1];
    // 1, 1 = locked - 1
    // 0, 1 = moving - 2
    // 0, 0 = open - 3
    // 1, 0 = def open - 4

    // checking sensor 1
    if (sensor1 == 1){
        // checking sensor 2
        if (sensor2 == 1){
            // state is locked
            state = 1;
        }
        else{
            // state is def open
            state = 4;
        }

    }
    else{
        if (sensor2 == 1){
            // state is moving
            state = 2;
        }
        else{
            // state is open
            state = 3;
        }
    }
    return state;
}


// function to check if locked 
int checkLock(){
    if (sensorState() != 1){
        return 0;
    }
    else
    {
        return 1;
    }
    
}


// drogue function
void drogue(){
    // turn motor on to open up the nose cone
    // checks sensor to see if the state is correct
    while(sensorState() != 4){
        //move back and forth a bit with the motor
        cout << "[INFO] NOT OPEN...\n";
    }
    Sleep(1000);
    // do not move motor
    cout << "[INFO] Drogue Launched\n";

}


// main parachute pull
void chute(){
    // pull linear actuator
    // check linear actuator position
    // make sure its open
    Sleep(1000);
    cout << "[INFO] Main Pulled\n";
}

// the ARMED function dealing with telemetrum
int waitForAvionics(){
    int * pinstatus;
    int iso_chute;
    int iso_drogue;
    int blank;

    pinstatus = gpio_check();
    iso_chute = pinstatus[ISO_CHUTE_Pin - 1];
    iso_drogue = pinstatus[ISO_DROGUE_Pin - 1];

    cout << "----ARMED----\n";
    if (iso_drogue == 1){
        drogue();
    }

    if (iso_chute == 1){
        chute();
        blank = 1;
    }

    return blank;
}


int main(int argc, char const *argv[]){
    int blank;
    while(1){
    
        if(checkLock()){

            while(1){
                blank = waitForAvionics();

                // exit loop after released the drogue remove this break statement to always be armed
                if (blank == 1){
                    cout << "[INFO] BLANKED\n";
                    Sleep(1000);
                    cout << "Refreshing...";
                    Sleep(1000);
                    break;
                }
            }
        }
        else{
            cout << "BUZZER ON\n";
        }
    }
    
    return 0;
}
