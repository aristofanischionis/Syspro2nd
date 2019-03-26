#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"


int newID(){
    printf("This is the newID function\n");
    return SUCCESS;
}

int removeID(){
    printf("This is the removeID function\n");
    return SUCCESS;
}