#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/Inotify.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/FileOperations.h"
#include "../../HeaderFiles/PipeOperations.h"

char* commonDir;
char* mirrorDir;
char* logfile;
int id;

void terminating(){
    char file[100];
    char* logdata;
    FILE* logfp;

    logdata = malloc(100);
    strcpy(logdata, "");

    if(commonDir && !commonDir[0]){
        printf("commonDir is empty\n");
        exit(ERROR);
    }
    if(mirrorDir && !mirrorDir[0]){
        printf("mirrorDir is empty\n");
        exit(ERROR);
    }
    printf(" Deleting: %s\n", mirrorDir);
    deleteFolder(mirrorDir);

    sprintf(file, "%s/%d.id", commonDir, id);
    printf(" Deleting %s\n", file);
    unlink(file);
    // write in log file that i am here
    logfp = fopen(logfile, "a");
    // write data in log
    sprintf(logdata, "Leaving id is: %d", id);
    fprintf(logfp, "%s\n", logdata);
    fclose(logfp);
    exit(SUCCESS);
}

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

int nameExists(char* filename){
    // struct stat buffer;   
    // return (stat (filename, &buffer) == 0);
    struct stat info;
    if(lstat(filename,&info) != 0) {
        if(errno == ENOENT) {
        //  doesn't exist
            return NO;
        } 
    }
    //so, it exists.
    return FILE_;
}

int writeIDfile(char* path, int id){
    char buf[MAX_PATH_LEN];
    char toMake[MAX_PATH_LEN];
    FILE* fp;
    sprintf(toMake, "%s/%d.id", realpath(path, buf), id);
    // printf("I am going to make file : %s \n", toMake);
    // check if file already exists
    if(nameExists(toMake) == FILE_){
        printf("File with id %d already exists \n", id);
        return ERROR;
    }
    // continue if it doesn't
    fp = fopen(toMake, "w");
    int pid = getpid();
    fprintf(fp, "%d", pid);
    fclose(fp);
    return SUCCESS;
}

int InputReader(int argc, char* argv[]){
    int n = argc;
    char* idchar;
    // int id;
    FILE* logfp;
    char* inputDir;
    // char* mirrorDir;
    char* b;
    int bSize;
    // char* logfile;
    struct stat st = {0};
    // init all the variables to be read as cmd line arguments
    idchar = (char*) malloc(10);
    commonDir = (char*) malloc(MAX_PATH_LEN);
    inputDir = (char*) malloc(MAX_PATH_LEN);
    mirrorDir = (char*) malloc(MAX_PATH_LEN);
    b = (char*) malloc(10);
    logfile = (char*) malloc(50);
    // 
    strcpy(idchar, "");
    strcpy(commonDir, "");
    strcpy(inputDir, "");
    strcpy(mirrorDir, "");
    strcpy(b, "");
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

    if(nameExists(inputDir) == NO){
        printf("input folder doesn't exist, exiting\n");
        return ERROR;
    }
    if(nameExists(mirrorDir) == DIR_){
        printf("mirror folder exists, exiting\n");
        return ERROR;
    }

    if (stat(commonDir, &st) == -1) {
        // mkdir(commonDir, 0700);
        makeFolder(commonDir);
    }
    if (stat(mirrorDir, &st) == -1) {
        // mkdir(mirrorDir, 0700);
        makeFolder(mirrorDir);
    }
    // before writing his id in common dir
    // set up handlers for sigint and sigquit
    signal(SIGINT, terminating);
    signal(SIGQUIT, terminating);

    if(writeIDfile(commonDir, id) == ERROR){
        return ERROR;
    }
    // write in log file that i am here
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[20];
    sprintf(logdata, "id: %d", id);
    fprintf(logfp, "%s\n", logdata);
    fclose(logfp);
    // first sync with all other processes
    syncr(id, commonDir, bSize, inputDir, mirrorDir, logfile);
    // begin monitoring commonDir
    inotifyCode(id, commonDir, bSize, inputDir, mirrorDir, logfile);

    return SUCCESS;
}
