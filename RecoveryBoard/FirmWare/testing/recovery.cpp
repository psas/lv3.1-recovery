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
bool checkLock(){
    if (sensorState() != 1){
        return false;
    }
    else
    {
        return true;
    }
    
}

// startup state
void startup(){
    checkLock();
}

int main(int argc, char const *argv[]){
    
    cout <<"The sensor is " <<sensorState()<<"\n";
    cout << "Locked?: " << checkLock()<<"\n";
    
    Sleep(100); // to allow the simulation program time to change the file
    
    return 0;
}
