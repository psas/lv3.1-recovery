#include<stdio.h>

void gather_input(char * buf){
    // int pin; 
    // int value;
    // printf("Enter Pin change -> [Pin] [Value 0/1]: ");
    // scanf("%d %d", pin, value);
    // printf("Changing Pin %d to %d", pin, value);

    gets(buf);
    printf("%s\n", buf);
}

void main(){
    char buf[100];
    while (1)
    {
      gather_input(buf);
    }
    
}