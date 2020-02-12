#include<iostream>
#include<fstream>

void gpio_check(){
    int pins[32];

    std::ifstream datafile;
    datafile.open("gpio_sim.txt");

    int count = 0;
    while(count<32){
    datafile >> pins[count];
    count++;
    }

    for(int i = 0; i < 32; i++){
        std::cout << pins[0] << "\n";
    }
    
    datafile.close();
}


int main(int argc, char const *argv[])
{   while (1)
{
    gpio_check();
}

    
    return 0;
}
