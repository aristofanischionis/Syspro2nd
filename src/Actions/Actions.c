#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/PipeOperations.h"
#include "../../HeaderFiles/FileOperations.h"

pid_t parentPid;
int signalsReceived = 0;
int doAgain = NO;

void handler(){
    signal(SIGUSR2, handler);
    printf("PARENT: I have received a SIGUSR2, FROM MY KIDDO\n");
    if(signalsReceived == 3){
        //maximum number achieved
        signalsReceived = 0;
        // do what i m  supposed
        doAgain = ERROR;
    }
    else {
        signalsReceived++;
        // call spawn kids
        doAgain = YES;
    }
}

int spawnKids(char* commonDir, int myID, int newID, char* inputDir, int b, char* mirrorDir, char* logfile){
    char SendData[FIFO_LEN];
    char ReceiveData[FIFO_LEN];
    char* filename;
    int flag ;
    filename = malloc(50);
    strcpy(filename, "");
    // setting the signal receiver
    signal(SIGUSR2, handler);
    
    // giving name to each fifo
    sprintf(SendData, "%s/%d_to_%d.fifo", commonDir, myID, newID);
    sprintf(ReceiveData, "%s/%d_to_%d.fifo", commonDir, newID, myID);
    //
    // printf("senddata ----> %s \n", SendData);
    // printf("recdata ------> %s \n", ReceiveData);
    parentPid = getpid();
    // forking the kids
    pid_t pid, wpid;
    for (int i = 0; i < 2; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            // do childern stuff
            if (i == 0)
            { // WRITER
                printf("I am WRITER of %d\n", myID);
                if(nameExists(SendData) != FILE_){
                    printf("%s desn't exist socreate \n", SendData);
                    mkfifo(SendData, 0666);
                }
                findFiles(inputDir, 0, SendData, b, inputDir);
                // all files are done so write the final 00 bytes, to let reader
                int fd;
                fd = open(SendData, O_WRONLY);
                writeFinal(fd);
                close(fd);
                // delete the fifo file from system
                unlink(SendData);
                exit(YES);
            }
            else if (i == 1)
            { // READER
                printf("I am READER of %d\n", myID);
                if(nameExists(ReceiveData) != FILE_){
                    printf("%s desn't exist so create \n", ReceiveData);
                    mkfifo(ReceiveData, 0666);
                }                
                flag = readPipe(myID, newID, ReceiveData, mirrorDir, logfile, b);

                unlink(ReceiveData);
                if(flag == YES){
                    exit(YES);
                }
                kill(parentPid,SIGUSR2);
                exit(NO);
            }
        }
    }
    // parent
    int status = 0;
    while ((wpid = wait(&status)) > 0){
        printf("status-------------->%d\n", status);
        if(WIFEXITED(status)){
            int exit_status = WEXITSTATUS(status);      
            printf("Exit status of %d was (%s)\n", (int)wpid,
                (exit_status == YES) ? "successful" : "not successful"); 
        }
    }
    if(doAgain == YES){
        // delete mirror
        if(deleteFolder(mirrorDir) == ERROR){
            perror("Folder cannot be deleted : \n");
            return ERROR;
        }
        struct stat st = {0};
        if (stat(mirrorDir, &st) == -1) {
            mkdir(mirrorDir, 0700);
        }
        // retry
        spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile);
    }
    else if (doAgain == ERROR){
        // reached 3 times
        return ERROR;
    }
    return SUCCESS;
}

int newID(char* commonDir , char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile){
    int retv;
    struct stat st = {0};
    char newFold[50];
    sprintf(newFold, "%s/%d", mirrorDir, newID);
    // make the corresponding folder in mirrorDir
    if (stat(newFold, &st) == -1) {
        mkdir(newFold, 0700);
    }
    retv = spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile);
    if(retv == ERROR){
        return ERROR;
    }
    return SUCCESS;
}

int removeID(){
    printf("This is the removeID function\n");
    return SUCCESS;
}

// call function newID for each id1.id file in common Dir except the id1 == myID
void syncr(int myID, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile){
    DIR *d;
    /* Open the directory specified by "commonDir". */
    d = opendir(commonDir);
    /* Check it was opened. */
    if (!d) {
        printf("Cannot open directory commonDir in findFiles\n");
        return;
    }
    printf("I am in syncr!!!!! \n");
    while (1) {
        struct dirent *entry;
        char *d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir(d);
        if (!entry) {
            /* There are no more entries in this directory, so break
            out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        
        // if it is the cur folder or the parent
        if (d_name[0] == '.') continue;
        char *dot = strrchr(d_name, '.');
        if (dot && !strcmp(dot, ".id")){
            // I found a filename that ends with .id
            
            int thisID = 0;
            sscanf(d_name, "%d.id", &thisID);
            if(thisID == myID) continue;
            // if it is a file do stuff
            printf("client with id %d found!\n", thisID);
            // call newID
            newID(commonDir, inputDir, myID, thisID, b, mirrorDir, logfile);
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir(d)) {
        fprintf(stderr, "Could not close '%s': %s\n", commonDir, strerror(errno));
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
}