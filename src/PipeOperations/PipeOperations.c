#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
// #include <limits.h>
#include <signal.h>
// #include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/PipeOperations.h"
#include "../../HeaderFiles/FileOperations.h"

pid_t parentPid;

void writeFinal(int fd){   
    char final[2] = "00";
    if (write(fd, final, 2) < 2){
        perror(" Error in Writing Final\n");
        kill(parentPid, SIGUSR2);
        exit(NO);
    }
}

void writeFile(char* filename, int parentfd, int b){
    FILE* fp;
    char *file;
    file = malloc(b);
    strcpy(file, "");
    // printf("the file ---> %s \n", filename);
    fp = fopen(filename, "r");
    if(fp == NULL){
        perror("Unable to open file: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // read 
    if(fread(file, 1, b, fp) < b){
        perror("read from file failed: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // printf("write file read : %s \n", file);
    if((write(parentfd, file, b)) < b){
        perror("Write to pipe failed: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    free(file);      
    fclose(fp);
}

void readFile(int parentfd, int b, char* newFile){
    FILE* newFD = fopen(newFile, "a");
    if (newFD == NULL) {
        perror("Failed to open file: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    char *file;
    file = malloc(b);
    strcpy(file, "");
    if (read(parentfd, file, b) < b){
        perror(" Error in reading pipe ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // printf("readFile has read : %s bytes %d , file %s\n", file, b, newFile);
    if(fprintf(newFD, "%s", file) < b){
        perror("write to newfile failed: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    free(file);
    fclose(newFD);
}

void writePipe(char* SendData, int b, char* actualPath, char* inputDir){
    parentPid = getppid();
    int fd, i, iter = 0, rem = 0;
    unsigned int size;
    unsigned short len;
    char* pathToBackup;
    pathToBackup = malloc(MAX_PATH_LEN);
    strcpy(pathToBackup, "");
    // open pipe
    printf("Before %s write end opens \n", SendData);
    fd = open(SendData, O_WRONLY);
    printf("After %s write end opens \n", SendData);
    // write len of file name
    // cut the first part of actual path that is not necessary in backup
    // name of interest to concat for the other process
    pathToBackup = formatBackupPath(inputDir, "", actualPath);
    len = strlen(pathToBackup);
    printf("WP->Size of name is %hu and name %s \n", len, pathToBackup);
    if (write(fd, &len, 2) < 2){
        perror(" Error in Writing in pipe1: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    if (write(fd, pathToBackup, len + 1) < (len + 1)){
        perror(" Error in Writing in pipe2: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // len of file
    size = (unsigned int)calculateFileSize(actualPath);

    if (write(fd, &size, 4) < 4){
        perror(" Error in Writing in pipe3: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    iter = size / b;
    rem = size - (iter*size);
    // printf("I calculated size %hu, iter %d, rem %d \n", size, iter, rem);
    for(i=0;i<iter;i++){
        // write part of the file data
        writeFile(actualPath, fd, b);
    }
    writeFile(actualPath, fd, rem);
    // wrote all of the file in fd
    // actual file in chunks size of b
    free(pathToBackup);
    close(fd);
}

int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b){
    // open pipe
    parentPid = getppid();
    int fd, i, iter, rem = 0;
    FILE* logfp;
    long nread = 0;
    unsigned int size = 0;
    unsigned short len = 0;
    char filename[300];
    char newFile[300];
    char s[3];
    printf("Before %s read end opens \n", ReceiveData);
    fd = open(ReceiveData, O_RDONLY);
    printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits, if they are 00, then exit successfully,
    // if they are not continue it is the len of next file
    while(1){
        if((nread = read(fd, s, 2)) < 2){
            perror(" Error in reading pipe1: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        if(!strcmp(s, "00")){
            printf("I read the 00 bytes from pipe so I m done\n");
            // successful
            close(fd);
            return YES;
        }
        else {
            len = atoi(s);
        }

        // if((nread = read(fd, &len, 2)) < 0){
        //     printf("I read %ld chars\n", nread);
        // }
        if((nread = read(fd, filename, len + 1)) < (len+1)){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe2: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }

        if((nread = read(fd, &size, 4)) < 0){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe3: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        // make new file in folder
        // in filename i have the actual path
        sprintf(newFile, "%s/%d/%s", mirrorDir, newID, filename);
        printf("file to be made is %s \n", newFile);
        makeFile(newFile);

        iter = size / b;
        rem = size - iter*size;
        // printf("size %hu, Rem is -> %d \n", size, rem);
        for(i=0;i<iter;i++){
            readFile(fd, b, newFile);
            // after i read a part of file open the new file in mirr and append data
        }
        readFile(fd, rem, newFile);
        
        logfp = fopen(logfile, "a");
        // write data in log
        char logdata[100];
        sprintf(logdata, "Name: %s, Size: %hu ", filename, size);
        printf(" ----------> to be written in log %s \n", logdata);
        fprintf(logfp, "%s\n", logdata);
        fclose(logfp);
    }
    return NO;
}
