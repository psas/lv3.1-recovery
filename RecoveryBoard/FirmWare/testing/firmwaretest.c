#include<stdio.h>

int * gpio_check(){
    FILE *fp = fopen("gpio_sim.txt", "r");
    static int pins[32];

    for (int i = 0; i < 32; i++)
    {
        fscanf(fp, "%d", &pins[i]);
    }

    // printf("\nCurrent GPIO\n");
    // for (int i = 0; i < 32; i++)
    // {
    //     printf("%d", pins[i]);
    // }
    fclose(fp);

    return pins;
}


int main(){
    while (1){
        
        int * status = gpio_check();
    }
    
    return 0;
}