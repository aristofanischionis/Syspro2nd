#include <stdio.h>
#include <stdlib.h> 
#include "./HeaderFiles/Input.h"

int main(int argc, char *argv[]){
    system("clear");
    printf("mirror_client Syspro 2nd Assignment Spring 2019 \n");
    if (InputReader(argc, argv) == ERROR) return 1;
    else return 0;
}