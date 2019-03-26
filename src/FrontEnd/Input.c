#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../../HeaderFiles/Input.h"

// parse command line args
void paramChecker(int n, char* argv[], char* toCheck, char** result){
    int i = 1;
    while( i<n ){
        if( strcmp(argv[i], toCheck) == 0 ){
            if( i < n - 1 ){
                if( argv[i+1][0] ==  '-' ){
                    printf("After %s flag a - was read\n", toCheck);
                    exit(1);
                }
                // printf("%s flag value is: %s\n", toCheck, argv[i+1]);
                strcpy(*result, argv[i+1]);
                return ;
            }
            else{
                printf("Param after %s flag was not given\n", toCheck);
                printf("exiting...\n");
            }
        }
        i++;
    }
}

int InputReader(int argc, char* argv[]){

    int n = argc;
    // init all the variables to be read as cmd line arguments
    char* idchar;
    idchar = (char*) malloc(10);
    strcpy(idchar, "");
    int id;
    //
    char* commonDir;
    commonDir = (char*) malloc(MAX_PATH_LEN);
    strcpy(commonDir, "");
    //
    char* inputDir;
    inputDir = (char*) malloc(MAX_PATH_LEN);
    strcpy(inputDir, "");
    //
    char* mirrorDir;
    mirrorDir = (char*) malloc(MAX_PATH_LEN);
    strcpy(mirrorDir, "");
    // 
    char* b;
    int bSize;
    b = (char*) malloc(10);
    strcpy(b, "");
    // 
    char* logfile;
    logfile = (char*) malloc(50);
    strcpy(logfile, "");

    // read all cmd arguments
    paramChecker(n, argv, "-n", &idchar);
    paramChecker(n, argv, "-c", &commonDir);
    paramChecker(n, argv, "-i", &inputDir);
    paramChecker(n, argv, "-m", &mirrorDir);
    paramChecker(n, argv, "-b", &b);
    paramChecker(n, argv, "-l", &logfile);

    id = atoi(idchar);
    bSize = atoi(b);

    free(b);
    free(idchar);

    // print them to be sure that everything is right
    if(bSize < 1 || id < 0){
        printf("Wrong parameters are given to the program\n");
        printf("Exiting now...\n");
        return ERROR;
    }
    // printf("So the params list is %d, %s, %s, %s, %d, %s \n", id, commonDir, inputDir, mirrorDir, bSize, logfile);
    
    return SUCCESS;
}