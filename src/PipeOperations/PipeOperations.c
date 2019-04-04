#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/PipeOperations.h"
#include "../../HeaderFiles/FileOperations.h"

pid_t parentPid;

void handle_alarm(){
    signal(SIGALRM, handle_alarm);
    printf("A read/write operation in fifo took longer than expected\n");
    kill(parentPid, SIGUSR1);
    exit(NO);
}

void writeFinal(int fd){   
    char final[3];
    strcpy(final, "00");
    alarm(30);
    if (write(fd, final, 3) < 0){
        perror(" Error in Writing Final\n");
        kill(parentPid, SIGUSR2);
        exit(NO);
    }
    alarm(0);
}

int writeFile(char* filename, int parentfd, int size, int b){
    int fd;
    char *file;
    int bytes = 0;
    ssize_t r = 0;
    file = malloc(b+1);
    strcpy(file, "");
    
    fd = open(filename, O_RDONLY);
    
    while(size > 0){
        strcpy(file, "");
        if(size < b){
            free(file);
            file = malloc(size+1);
            strcpy(file, "");
            
            if((r = read(fd, file, size)) < 0){
                perror("read from file failed: ");
                kill(parentPid,SIGUSR2);
                exit(NO);
            }
            file[r] = '\0';
            alarm(30);
            if((r = write(parentfd, file, size)) < 0){
                perror("Write to pipe failed: ");
                kill(parentPid,SIGUSR2);
                exit(NO);
            }
            alarm(0);
            //
            bytes += (int)r;
            size = 0;
            break;
        }
        // else if we need more than one loops to read and write files
        if((r = read(fd, file, b)) < 0){
            perror("read from file failed: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        file[r] = '\0';
        alarm(30);
        if((r = write(parentfd, file, b)) < 0){
            perror("Write to pipe failed: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        alarm(0);
        //
        bytes += (int)r;
        size = size - b;
    }
    free(file);      
    close(fd);
    return bytes;
}

int readFile(int parentfd, int size, char* newFile, int b){
    FILE* newFD = fopen(newFile, "w");
    char *file;
    int bytes = 0;
    ssize_t r = 0;
    file = malloc(b+1);
    strcpy(file, "");

    if (newFD == NULL) {
        perror("Failed to open file: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    while(size > 0){
        if(size < b){
            free(file);
            file = malloc(size+1);
            strcpy(file, "");
            alarm(30);
            if ((r = read(parentfd, file, size)) < 0){
                perror(" Error in reading pipe ");
                kill(parentPid,SIGUSR2);
                exit(NO);
            }
            file[r] = '\0';
            //
            bytes += (int)r;
            alarm(0);
            if(fprintf(newFD, "%s", file) < 0){
                perror("write to newfile failed: ");
                kill(parentPid,SIGUSR2);
                exit(NO);
            }
            size = 0;
            break;
        }
        alarm(30);
        if ((r = read(parentfd, file, b)) < 0){
            perror(" Error in reading pipe ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        file[r] = '\0';
        //
        bytes += (int)r;
        alarm(0);
        if(fprintf(newFD, "%s", file) < 0){
            perror("write to newfile failed: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        size = size - b;
    }

    free(file);
    fclose(newFD);
    return bytes;
}

void writePipe(char* SendData, int b, char* actualPath, char* inputDir, char* logfile){
    // Install alarm handler
    signal(SIGALRM, handle_alarm);
    parentPid = getppid();
    // printf("I am writePipe and my dad %d and i setted up the alarm handler\n", parentPid);
    int fd;
    FILE* logfp;
    int bytesWritten = 0;
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
    // printf("Before %s write end opens \n", SendData);
    alarm(30);
    fd = open(SendData, O_WRONLY);
    alarm(0);
    // printf("After %s write end opens \n", SendData);
    // write len of file name
    // cut the first part of actual path that is not necessary in backup
    // name of interest to concat for the other process
    pathToBackup = formatBackupPath(inputDir, "", actualPath);
    l = strlen(pathToBackup);
    sprintf(len, "%hu", l);
    printf("WRITER->Size of name is %s and name %s \n", len, pathToBackup);
    alarm(30);
    if (write(fd, len, 3) < 0){
        perror(" Error in Writing in pipe1: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    alarm(0);
    alarm(30);
    if (write(fd, pathToBackup, l + 1) < 0){
        perror(" Error in Writing in pipe2: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    alarm(0);
    // len of file
    size = (unsigned int)calculateFileSize(actualPath);
    sprintf(s, "%u", size);
    alarm(30);
    if (write(fd, s, 5) < 0){
        perror(" Error in Writing in pipe3: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    alarm(0);
    bytesWritten = writeFile(actualPath, fd, size, b);
    // wrote all of the file in fd
    // actual file in chunks size of b
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[200];
    sprintf(logdata, "File Written: %s, Bytes: %d ", actualPath, bytesWritten);
    // printf(" ----------> to be written in log %s \n", logdata);
    fprintf(logfp, "%s\n", logdata);
    fclose(logfp);
    free(pathToBackup);
    close(fd);
}

int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b){
    // Install alarm handler
    signal(SIGALRM, handle_alarm);
    parentPid = getppid();
    // printf("I am readPipe and my dad %d and i setted up the alarm handler\n", parentPid);
    int fd;
    FILE* logfp;
    long nread = 0;
    unsigned int size = 0;
    unsigned short len = 0;
    int bytesRead = 0;
    char s[5];
    char l[3];
    char* filename;
    char* newFile;
    filename = malloc(MAX_PATH_LEN);
    newFile = malloc(MAX_PATH_LEN);
    // char s[3];
    // printf("Before %s read end opens \n", ReceiveData);
    alarm(30);
    fd = open(ReceiveData, O_RDONLY);
    alarm(0);
    // printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits, if they are 00, then exit successfully,
    // if they are not continue it is the len of next file
    while(1){
        strcpy(s, "");
        strcpy(l, "");
        strcpy(filename, "");
        strcpy(newFile, "");
        alarm(30);
        if((nread = read(fd, l, 3)) < 0){
            perror(" Error in reading pipe1: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        alarm(0);

        len = atoi(l);
        if(len == 0){
            printf("I read the 00 bytes from pipe so I m done\n");
            // successful
            free(filename);
            free(newFile);
            close(fd);
            return YES;
        }
        alarm(30);
        if((nread = read(fd, filename, len + 1)) < 0){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe2: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        alarm(0);
        alarm(30);
        if((nread = read(fd, s, 5)) < 0){
            printf("I read %ld chars\n", nread);
            perror(" Error in reading pipe3: ");
            kill(parentPid,SIGUSR2);
            exit(NO);
        }
        alarm(0);
        // printf("size is %s \n", s);
        size = atoi(s);
        // make new file in folder
        // in filename i have the actual path
        sprintf(newFile, "%s/%d/%s", mirrorDir, newID, filename);
        // printf("file to be made is %s \n", newFile);
        makeFile(newFile);
        printf("READER->name is %s and size %d \n", newFile, size);

        bytesRead = readFile(fd, size, newFile, b);
        logfp = fopen(logfile, "a");
        // write data in log
        char logdata[200];
        sprintf(logdata, "File Read: %s, Bytes: %d ", newFile, bytesRead);
        // printf(" ----------> to be written in log %s \n", logdata);
        fprintf(logfp, "%s\n", logdata);
        fclose(logfp);
    }
    return NO;
}
