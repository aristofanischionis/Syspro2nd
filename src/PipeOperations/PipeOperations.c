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
    char final[3];
    strcpy(final, "00");
    if (write(fd, final, 3) < 0){
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
    if(fread(file, 1, b, fp) < 0){
        perror("read from file failed: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // printf("write file read : %s \n", file);
    if((write(parentfd, file, b+1)) < 0){
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
    file = malloc(b+1);
    strcpy(file, "");
    if (read(parentfd, file, b+1) < 0){
        perror(" Error in reading pipe ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // printf("readFile has read : %s bytes %d , file %s\n", file, b, newFile);
    if(fprintf(newFD, "%s", file) < 0){
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
    char s[5];
    char len[3];
    unsigned short l = 0;
    unsigned int size = 0;
    char* pathToBackup;
    pathToBackup = malloc(MAX_PATH_LEN);
    strcpy(pathToBackup, "");
    strcpy(s, "");
    strcpy(len, "");
    // open pipe
    printf("Before %s write end opens \n", SendData);
    fd = open(SendData, O_WRONLY);
    printf("After %s write end opens \n", SendData);
    // write len of file name
    // cut the first part of actual path that is not necessary in backup
    // name of interest to concat for the other process
    pathToBackup = formatBackupPath(inputDir, "", actualPath);
    l = strlen(pathToBackup);
    sprintf(len, "%hu", l);
    printf("WP->Size of name is %s and name %s \n", len, pathToBackup);
    if (write(fd, len, 3) < 0){
        perror(" Error in Writing in pipe1: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    printf("-> %s , l == %hu \n", pathToBackup, l);
    if (write(fd, pathToBackup, l + 1) < 0){
        perror(" Error in Writing in pipe2: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // len of file
    size = (unsigned int)calculateFileSize(actualPath);
    sprintf(s, "%u", size);
    printf("Size is %s \n", s);

    if (write(fd, s, 5) < 0){
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
    char s[5];
    char l[3];
    char* filename;
    char* newFile;
    filename = malloc(MAX_PATH_LEN);
    newFile = malloc(MAX_PATH_LEN);
    // char s[3];
    printf("Before %s read end opens \n", ReceiveData);
    fd = open(ReceiveData, O_RDONLY);
    printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits, if they are 00, then exit successfully,
    // if they are not continue it is the len of next file
    while(1){
        strcpy(s, "");
        strcpy(l, "");
        strcpy(filename, "");
        strcpy(newFile, "");
        if((nread = read(fd, l, 3)) < 0){
            perror(" Error in reading pipe1: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        // if(!strcmp(s, "00")){
        //     printf("I read the 00 bytes from pipe so I m done\n");
        //     // successful
        //     close(fd);
        //     return YES;
        // }
        // else {
        //     len = atoi(s);
        // }
        len = atoi(l);
        if(len == 0){
            printf("I read the 00 bytes from pipe so I m done\n");
            // successful
            free(filename);
            free(newFile);
            close(fd);
            return YES;
        }
        printf("i read len -----------> %d\n", len);
        // if((nread = read(fd, &len, 2)) < 0){
        //     printf("I read %ld chars\n", nread);
        // }
        if((nread = read(fd, filename, len + 1)) < 0){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe2: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        printf("-----------------> %s\n", filename);
        if((nread = read(fd, s, 5)) < 0){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe3: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        printf("size is %s \n", s);
        size = atoi(s);
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
