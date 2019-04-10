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
#include "../../HeaderFiles/Encryption.h"

pid_t parentPid;

void handle_alarm(){
    signal(SIGALRM, handle_alarm);
    printf("A read/write operation in fifo took longer than expected\n");
    kill(parentPid, SIGUSR2);
    exit(NO);
}

void writeFinal(int fd){
    char buffer[5];
    int final = 0;
    sprintf(buffer, "%d", final);
    myWrite(fd, buffer, 4);
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
        perror("Failed to open file:(readFile) ");
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
            alarm(0);
            file[r] = '\0';
            //
            bytes += (int)r;
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

int myRead(int fd, char* result, int bytes){
    int charsRead = 0;
    char* buffer = malloc(MAX_PATH_LEN);
    strcpy(buffer, "");
    alarm(30);
    if((charsRead = read(fd, buffer, bytes)) < 0){
        printf("I read %d chars\n", charsRead);
        perror(" Error in reading pipe: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    alarm(0);
    buffer[bytes] = '\0';
    strcpy(result, buffer);
    // printf("R result is %s and bytes: %d chars %d \n ", result, bytes, charsRead);
    free(buffer);
    return charsRead;
}

int myWrite(int fd, char* toWrite, int bytes){
    int charsWrite = 0;
    char buffer[MAX_PATH_LEN];
    strcpy(buffer, toWrite);
    alarm(30);
    if ((charsWrite = write(fd, buffer, bytes)) < 0){
        printf("I wrote %d chars\n", charsWrite);
        perror(" Error in Writing in pipe: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    alarm(0);
    // printf("W buffer is %s and bytes: %d, charsWrite %d \n ", buffer, bytes, charsWrite);
    return charsWrite;
}


void writePipe(int fd, int b, char* actualPath, char* inputDir, char* logfile){
    // Install alarm handler
    signal(SIGALRM, handle_alarm);
    parentPid = getppid();
    // printf("I am writePipe and my dad %d and i setted up the alarm handler\n", parentPid);
    int isDir = NO;
    char* buffer;
    FILE* logfp;
    ssize_t nwrite = 0;
    int bytesWritten = 0;
    int metaWritten = 0;
    int len = 0;
    int size = 0;
    char* pathToBackup;
    pathToBackup = malloc(MAX_PATH_LEN);
    buffer = malloc(MAX_PATH_LEN);
    strcpy(buffer, "");
    strcpy(pathToBackup, "");
    // check if dir or file
    DIR* dir = opendir(actualPath);
    if(dir){
        // it is a dir
        isDir = YES;
        closedir(dir);
    }
    // open pipe
    // printf("Before %s write end opens \n", SendData);
    // alarm(30);
    // fd = open(SendData, O_WRONLY);
    // if(fd < 0){
    //     perror(" error in WritePipe: ");
    //     exit(NO);
    // }
    // alarm(0);
    // printf("After %s write end opens \n", SendData);
    // write len of file name
    // cut the first part of actual path that is not necessary in backup
    // name of interest to concat for the other process
    pathToBackup = formatBackupPath(inputDir, "", actualPath);
    len = strlen(pathToBackup);
    // printf("WRITER->Size of name is %s and name %s \n", len, pathToBackup);
    if(isDir == YES){
        //send an identifier l == -1
        int folderLen = -1;
        strcpy(buffer, "");
        sprintf(buffer, "%d", folderLen);
        // alarm(30);
        // if ((nwrite = write(fd, buffer, 2)) < 0){
        //     perror(" Error in Writing in pipe4: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        myWrite(fd, buffer, 4);
        // write the name len
        strcpy(buffer, "");
        sprintf(buffer, "%d", len);

        // alarm(30);
        // if ((nwrite = write(fd, buffer, 2)) < 0){
        //     perror(" Error in Writing in pipe5: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        myWrite(fd, buffer, 4);
        // write name of folder
        strcpy(buffer, "");
        strcpy(buffer, pathToBackup);
        // alarm(30);
        // if ((nwrite= write(fd, buffer, len)) < 0){
        //     perror(" Error in Writing in pipe6: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        myWrite(fd, buffer, len);
        free(pathToBackup);
        // close(fd);
        return;
    }

    // if it is a file
    strcpy(buffer, "");
    sprintf(buffer, "%d", len);
    // alarm(30);
    // if ((nwrite = write(fd, buffer, 2)) < 0){
    //     perror(" Error in Writing in pipe1: ");
    //     kill(parentPid,SIGUSR2);
    //     exit(NO);
    // }
    // alarm(0);
    nwrite = myWrite(fd, buffer, 4);
    metaWritten += (int)nwrite;
    strcpy(buffer, "");
    strcpy(buffer, pathToBackup);
    // alarm(30);
    // if ((nwrite= write(fd, buffer, len)) < 0){
    //     perror(" Error in Writing in pipe2: ");
    //     kill(parentPid,SIGUSR2);
    //     exit(NO);
    // }
    // alarm(0);
    nwrite = myWrite(fd, buffer, len);
    metaWritten += (int)nwrite;
    // len of file
    size = calculateFileSize(actualPath);
    // sprintf(s, "%d", size);
    // printf("---------> size %s \n", s);
    strcpy(buffer, "");
    sprintf(buffer, "%d", size);
    // alarm(30);
    // if ((nwrite = write(fd, buffer, 4)) < 0){
    //     perror(" Error in Writing in pipe3: ");
    //     kill(parentPid,SIGUSR2);
    //     exit(NO);
    // }
    // alarm(0);
    nwrite = myWrite(fd, buffer, 4);
    metaWritten += (int)nwrite;
    //
    bytesWritten = writeFile(actualPath, fd, size, b);
    // wrote all of the file in fd
    // actual file in chunks size of b
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[200];
    sprintf(logdata, "File Written: %s, Bytes: %d Meta Written: %d", actualPath, bytesWritten, metaWritten);
    fprintf(logfp, "%s\n", logdata);
    fflush(logfp);
    fclose(logfp);
    free(pathToBackup);
    // close(fd);
}

int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b, char* passPhrase){
    // Install alarm handler
    signal(SIGALRM, handle_alarm);
    parentPid = getppid();
    // printf("I am readPipe and my dad %d and i setted up the alarm handler\n", parentPid);
    char* buffer;
    int fd;
    FILE* logfp;
    ssize_t nread = 0;
    int size = 0;
    int len = 0;
    int bytesRead = 0;
    int metaRead = 0;
    char* filename;
    char* newFile;
    buffer = malloc(MAX_PATH_LEN);
    filename = malloc(MAX_PATH_LEN);
    newFile = malloc(MAX_PATH_LEN);
    // printf("Before %s read end opens \n", ReceiveData);
    // alarm(30);
    fd = open(ReceiveData, O_RDONLY);
    if(fd < 0){
        perror(" error in WritePipe: ");
        exit(NO);
    }
    // alarm(0);
    // printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits, if they are 00, then exit successfully,
    // if they are not continue it is the len of next file
    while(1){
        strcpy(filename, "");
        strcpy(newFile, "");
        strcpy(buffer, "");
        // alarm(30);
        // if((nread = read(fd, buffer, 2)) < 0){
        //     perror(" Error in reading pipe1: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        // buffer[2] = '\0';
        nread = myRead(fd, buffer, 4);
        len = atoi(buffer);
        if(len == 0){
            printf("I read the 00 bytes from pipe so I m done\n");
            // successful
            free(filename);
            free(newFile);
            close(fd);
            return YES;
        }
        if(len == -1){
            // folder
            char* folderName;
            char* newFolder;
            int folderNameLen = 0;
            folderName = malloc(MAX_PATH_LEN);
            newFolder = malloc(MAX_PATH_LEN);
            strcpy(newFolder, "");
            // alarm(30);
            // if((nread = read(fd, buffer, 2)) < 0){
            //     perror(" Error in reading pipe4: ");
            //     kill(parentPid,SIGUSR2);
            //     exit(NO);
            // }
            // alarm(0);
            // buffer[2] = '\0';
            myRead(fd, buffer, 4);
            folderNameLen = atoi(buffer);
            // read folder path

            // alarm(30);
            // if((nread = read(fd, buffer, folderNameLen)) < 0){
            //     printf("I read %ld chars\n", nread);
            //     perror(" Error in reading pipe2: ");
            //     kill(parentPid,SIGUSR2);
            //     exit(NO);
            // }
            // alarm(0);
            // buffer[folderNameLen] = '\0';
            myRead(fd, buffer, folderNameLen);
            strcpy(folderName, buffer);
            // make the path to backup
            sprintf(newFolder, "%s/%d/%s", mirrorDir, newID, folderName);
            makeFolder(newFolder);

            continue;
        }
        metaRead += (int)nread;

        // alarm(30);
        // if((nread = read(fd, buffer, len )) < 0){
        //     printf("I read %ld chars\n", nread);
        //     perror(" Error in reading pipe2: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        nread = myRead(fd, buffer, len);
        metaRead += (int)nread;
        // buffer[len] = '\0';
        strcpy(filename, buffer);

        // alarm(30);
        // if((nread = read(fd, buffer, 4)) < 0){
        //     printf("I read %ld chars\n", nread);
        //     perror(" Error in reading pipe3: ");
        //     kill(parentPid,SIGUSR2);
        //     exit(NO);
        // }
        // alarm(0);
        nread = myRead(fd, buffer, 4);
        metaRead += (int)nread;
        // buffer[4] = '\0';
        size = atoi(buffer);
        // make new file in folder
        // in filename i have the actual path
        if(!strcmp(filename, "")){
            printf("----------------------------------->PROBLEMMMMMMMMMMMMMMM!!!!!!!!!\n");
            continue;
        }
        sprintf(newFile, "%s/%d/%s", mirrorDir, newID, filename);
        // printf("file to be made is %s \n", newFile);
        makeFile(newFile);
        // printf("READER->name is %s and size %d \n", newFile, size);

        bytesRead = readFile(fd, size, newFile, b);
        logfp = fopen(logfile, "a");
        // write data in log
        char logdata[200];
        sprintf(logdata, "File Read: %s, Bytes: %d Meta Read: %d", newFile, bytesRead, metaRead);
        fprintf(logfp, "%s\n", logdata);
        fflush(logfp);
        fclose(logfp);
        metaRead = 0;

        // call the decrypt function
        decryptFile(passPhrase, newFile);
        // delete newfile
        unlink(newFile); 
    }
    return NO;
}