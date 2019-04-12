#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/Inotify.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/FileOperations.h"
#include "../../HeaderFiles/PipeOperations.h"
#include "../../HeaderFiles/Encryption.h"

// these are global so that the terminating function can use them
char* commonDir;
char* mirrorDir;
char* logfile;
int id;
// this is the flag you need to change OFF and ON
// if OFF -> program runs faster but more insecurely
// if ON -> program runs slower but more securely
int ENCRYPTION_MODE = ON;

void terminating(){
    // handler for SIGQUIT, SIGINIT
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
    fflush(logfp);
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

// checks if this filename exists
int nameExists(char* filename){
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

// it is used to make a random passphrase for each client
void randomStringGenerator(size_t length, char *randomString) { // const size_t length, supra
    srand(time(NULL));
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int n, l, key;
    if(length){
        if(randomString){
            l = (int) (sizeof(charset) -1);
            for (n = 0; n < length; n++) {
                key = rand() % l;          // per-iteration instantiation
                randomString[n] = charset[key];
            }
            randomString[length] = '\0';
        }
    }
}

// writes my pid in file and the public key alias if encryption is on
int writeIDfile(char* path){
    char buf[MAX_PATH_LEN];
    char toMake[MAX_PATH_LEN];
    FILE* fp;
    sprintf(toMake, "%s/%d.id", realpath(path, buf), id);
    // check if file already exists
    if(nameExists(toMake) == FILE_){
        printf("File with id %d already exists \n", id);
        return ERROR;
    }
    // continue if it doesn't
    fp = fopen(toMake, "w");
    int pid = getpid();
    if(ENCRYPTION_MODE == ON){
        // this is the alias other people will need in order to encrypt messages
        // that only this user can decrypt
        fprintf(fp, "%d client%d@syspro.gr\n", pid, id);
    }
    else{
        fprintf(fp, "%d\n", pid);
    }
    fclose(fp);
    return SUCCESS;
}

int InputReader(int argc, char* argv[]){
    int n = argc;
    char* idchar;
    FILE* logfp;
    char* inputDir;
    char* b;
    int bSize;
    char* passPhrase;
    struct stat st = {0};
    // init all the variables to be read as cmd line arguments
    passPhrase = (char*) malloc(11);
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
    strcpy(passPhrase, "");

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
        makeFolder(commonDir);
    }

    if (stat(mirrorDir, &st) == -1) {
        makeFolder(mirrorDir);
    }

    // before writing his id in common dir
    // set up handlers for sigint and sigquit
    signal(SIGINT, terminating);
    signal(SIGQUIT, terminating);

    if(ENCRYPTION_MODE == ON){
        // make my encryption keys
        randomStringGenerator(10, passPhrase);
        printf("I am client %d and have pass: %s\n", id, passPhrase);
        generateKeys(id, passPhrase);
    }
    else{
        printf("I am client %d, with ENCRYPTION_MODE : OFF\n", id);
    }
    
    // write id and alias(public key) in commonDir file
    if(writeIDfile(commonDir) == ERROR){
        return ERROR;
    }
    // write in log file that i am here
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[20];
    sprintf(logdata, "id: %d", id);
    fprintf(logfp, "%s\n", logdata);
    fflush(logfp);
    fclose(logfp);
    // first sync with all other processes
    printf("Initial Syncronization Started! \n");
    syncr(id, commonDir, bSize, inputDir, mirrorDir, logfile, passPhrase);
    printf("Initial Syncronization finished!\n");
    // begin monitoring commonDir
    inotifyCode(id, commonDir, bSize, inputDir, mirrorDir, logfile, passPhrase);

    return SUCCESS;
}
