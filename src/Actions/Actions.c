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
#include "../../HeaderFiles/Encryption.h"

pid_t parentPid;
volatile sig_atomic_t signalsReceived = 0;
volatile sig_atomic_t STOP = NO;
volatile sig_atomic_t alarmSIG = OFF;
volatile sig_atomic_t WRITER = ALIVE;
volatile sig_atomic_t READER = ALIVE;

void handler2()
{
    // WRITER is dead
    signal(SIGUSR2, handler2);
    signalsReceived++;
    WRITER = DEAD;
    printf("PARENT: %d I have received a SIGUSR2\n", (int)parentPid);
    if (signalsReceived == 3)
    {
        //maximum number achieved
        signalsReceived = 0;

        STOP = YES;
    }
}

void handler1()
{
    // READER is dead
    signal(SIGUSR1, handler1);
    signalsReceived++;
    READER = DEAD;
    printf("PARENT: %d I have received a SIGUSR1\n", (int)parentPid);
    if (signalsReceived == 3)
    {
        //maximum number achieved
        signalsReceived = 0;
        STOP = YES;
    }
}

void handle_alarm()
{
    signal(SIGALRM, handle_alarm);
    signalsReceived++;
    printf("PARENT: %d I have received a SIGALARM\n", (int)parentPid);
    alarmSIG = ON;
}

void WRITER_process(char *SendData, int newID, char *commonDir, char *inputDir, int b, char *logfile, char* passPhrase)
{
    int fd;
    char *recepientEmail;
    recepientEmail = malloc(30);
    strcpy(recepientEmail, "");
    // WRITER
    signal(SIGPIPE, handle_SIGPIPE);
    // writerPid = getpid();
    // printf("I am WRITER with pid %d\n", writerPid);
    if (nameExists(SendData) != FILE_)
    {
        // printf("%s desn't exist so create \n", SendData);
        mkfifo(SendData, 0666);
    }
    if(strcmp(passPhrase, "")){
        // ENCRYPTION_MODE is ON
        // find the email "alias for public key"
        if (findEmail(commonDir, newID, recepientEmail) == ERROR)
        {
            fprintf(stderr, "Couldn't find Email of id: %d \n", newID);
            kill(parentPid, SIGUSR2);
            exit(NO);
        }
    }
    fd = open(SendData, O_WRONLY);
    if (fd < 0)
    {
        perror(" error in WritePipe: ");
        exit(NO);
    }
    findFiles(inputDir, 0, fd, b, inputDir, logfile, recepientEmail);
    // all files are done so write the final 00 bytes, to let reader
    writeFinal(fd);
    close(fd);
    // delete the fifo file from system
    unlink(SendData);
    exit(YES);
}

void READER_process(char* ReceiveData, int myID, int newID, char* mirrorDir, char* logfile, int b, char* passPhrase){
    int flag;
    // READER
    // readerPid = getpid();
    // printf("I am READER with pid %d\n", readerPid);
    if (nameExists(ReceiveData) != FILE_){
        // printf("%s desn't exist so create \n", ReceiveData);
        mkfifo(ReceiveData, 0666);
    }
    flag = readPipe(myID, newID, ReceiveData, mirrorDir, logfile, b, passPhrase);

    unlink(ReceiveData);
    if (flag == YES)
    {
        exit(YES);
    }
    kill(parentPid, SIGUSR1);
    exit(NO);
}

int spawnKids(char *commonDir, int myID, int newID, char *inputDir, int b, char *mirrorDir, char *logfile, char *passPhrase, int r, int w)
{
    char SendData[FIFO_LEN];
    char ReceiveData[FIFO_LEN];
    sprintf(SendData, "%s/%d_to_%d.fifo", commonDir, myID, newID);
    sprintf(ReceiveData, "%s/%d_to_%d.fifo", commonDir, newID, myID);
    // setting the signal receivers
    signal(SIGUSR2, handler2);
    signal(SIGUSR1, handler1);
    // Install alarm handler
    signal(SIGALRM, handle_alarm);
    parentPid = getpid();
    // forking the kids
    // pid_t pid, wpid;
    pid_t pid[2];
    for (int i = 0; i < 2; i++)
    {
        // pid = fork();
        if ((pid[i] = fork()) == 0)
        {
            // do childern stuff
            if ((i == 0) && (w == YES))
            {
                WRITER_process(SendData, newID, commonDir, inputDir, b, logfile, passPhrase);
            }
            else if ((i == 1) && (r == YES))
            {
                READER_process(ReceiveData, myID, newID, mirrorDir, logfile, b, passPhrase);
            }
        }
    }
    // parent
    int exit_status;
    int stat;
    for (int i=0; i<2; i++) 
    { 
        pid_t cpid = waitpid(pid[i], &stat, 0); 
        if (WIFEXITED(stat)){
            exit_status = WEXITSTATUS(stat);
            printf("Exit status of client's %d %s was (%s)\n",
                myID,
                (cpid == pid[0]) ? "WRITER" : "READER",
                (exit_status == YES) ? "successful" : "not successful");
        }
    } 
    if (exit_status == YES){
        return SUCCESS;
    }
    if ((alarmSIG == ON) && (STOP == NO))
    {
        alarmSIG = OFF;
    }
    if (STOP == NO)
    {
        if (WRITER == DEAD)
        {
            WRITER = ALIVE;
            sleep(2);
            spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile, passPhrase, NO, YES);
        }
        if (READER == DEAD)
        {
            READER = ALIVE;
            sleep(2);
            spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile, passPhrase, YES, NO);
        }
    }
    else if (STOP == YES)
    {
        // reached 3 times
        if (WRITER == DEAD)
        {
            unlink(SendData);
        }
        if (READER == DEAD)
        {
            unlink(ReceiveData);
        }
        return ERROR;
    }
    return SUCCESS;
}

int newID(char *commonDir, char *inputDir, int myID, int newID, int b, char *mirrorDir, char *logfile, char *passPhrase)
{
    int retv;
    struct stat st = {0};
    char newFold[MAX_PATH_LEN];
    sprintf(newFold, "%s/%d", mirrorDir, newID);
    // make the corresponding folder in mirrorDir
    if (stat(newFold, &st) == -1)
    {
        mkdir(newFold, 0700);
    }
    retv = spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile, passPhrase, YES, YES);
    if (retv == ERROR)
    {
        return ERROR;
    }
    return SUCCESS;
}

int removeID(char *inputDir, int deletedID, char *mirrorDir)
{

    char tobeDEL[MAX_PATH_LEN];
    sprintf(tobeDEL, "%s/%d", mirrorDir, deletedID);
    printf("Now removing %s \n", tobeDEL);
    // forks a kid and execs rm -rf of the given folder
    if (deleteFolder(tobeDEL) == ERROR)
    {
        return ERROR;
    }
    return SUCCESS;
}

// call function newID for each id1.id file in common Dir except the id1 == myID
void syncr(int myID, char *commonDir, int b, char *inputDir, char *mirrorDir, char *logfile, char *passPhrase)
{
    DIR *d;
    /* Open the directory specified by "commonDir". */
    d = opendir(commonDir);
    /* Check it was opened. */
    if (!d)
    {
        printf("Cannot open directory commonDir in syncr\n");
        return;
    }
    // printf("I am in syncronizing mode \n");
    while (1)
    {
        struct dirent *entry;
        char *d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir(d);
        if (!entry)
        {
            /* There are no more entries in this directory, so break
            out of the while loop. */
            break;
        }
        d_name = entry->d_name;

        // if it is the cur folder or the parent
        if (d_name[0] == '.')
            continue;
        char *dot = strrchr(d_name, '.');
        if (dot && !strcmp(dot, ".id"))
        {
            // I found a filename that ends with .id
            int thisID = 0;
            sscanf(d_name, "%d.id", &thisID);
            if (thisID == myID)
                continue;
            // if it is a file do stuff
            printf("client with id %d found!\n", thisID);
            // call newID
            pid_t pid;
            pid = fork();
            if (pid == 0)
            {
                newID(commonDir, inputDir, myID, thisID, b, mirrorDir, logfile, passPhrase);
                exit(YES);
            }
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir(d))
    {
        fprintf(stderr, "Could not close '%s': %s\n", commonDir, strerror(errno));
        exit(NO);
    }
}