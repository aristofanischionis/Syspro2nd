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

// this is the default signal when the reader quits and writer stays in the pipe 
// if this happens the writer should stop and send signal to parent
void handle_SIGPIPE(){
    signal(SIGPIPE, handle_SIGPIPE);
    printf("WRITER got a SIGPIPE error, sending signal to father\n");
    kill(getppid(), SIGUSR2);
    exit(NO);
}

// final "00" bytes written in pipe
void writeFinal(int fd){
    short int final = 0;
    myWrite(fd, &final, sizeof(short int));
}

// write context of file in pipe
// parentfd is the fd of the pipe where i write data
int writeFile(char* filename, int parentfd, int size, int b){
    int fd;
    char *file;
    int bytes = 0;
    int r = 0;
    file = malloc(b+1);
    strcpy(file, "");	
    fd = open(filename, O_RDONLY);
    
    while(size > 0){
        free(file);		
	    file = malloc(size+1);		
        strcpy(file, "");
        if(size < b){
            // reading size bytes from file
            if((r = read(fd, file, size)) < 0){
                perror("read from file failed: ");
                kill(getppid(), SIGUSR2);
                exit(NO);
            }
            // finalize string
            file[r] = '\0';
            // write size bytes to pipe
            r = myWrite(parentfd, file, size);
            //
            bytes += r;
            size = 0;
            // finished
            break;
        }
        // else if we need more than one loops to read and write files
        if((r = read(fd, file, b)) < 0){
            perror("read from file failed: ");
            kill(getppid(), SIGUSR2);
            exit(NO);
        }
        file[r] = '\0';
        
        r = myWrite(parentfd, file, b);
        bytes += r;
        size -= b;
    }
    free(file);      
    close(fd);
    return bytes;
}

// reading size bytes from parentfd(pipe) in chunks of b bytes
// and writing them to newFile
int readFile(int parentfd, int size, char* newFile, int b){
    FILE* newFD = fopen(newFile, "w");
    char* file;
    int bytes = 0;
    int r = 0;
    file = malloc(b+1);

    if (newFD == NULL ) {
        perror("Failed to open file:(readFile) ");
        kill(getppid(), SIGUSR1);
        exit(NO);
    }

    while(size > 0){
        if(size < b){
            free(file);
            file = malloc(size + 1);
            strcpy(file, "");
            // reading from pipe size bytes
            r = myRead(parentfd, file, size);
            // finalizing string
            file[r] = '\0';
            //
            bytes += r;
            // writing to newFile
            if(fprintf(newFD, "%s", file) < 1){
                perror("write to newfile failed: ");
                kill(getppid(), SIGUSR1);
                exit(NO);
            }
            fflush(newFD);
            size = 0;
            // finished
            break;
        }
        r = myRead(parentfd, file, b);
        file[r] = '\0';
        if(fprintf(newFD, "%s", file) < 1){
            perror("write to newfile failed: ");
            kill(getppid(), SIGUSR1);
            exit(NO);
        }
        //
        bytes += r;
        size -= b;
    }
    free(file);
    fclose(newFD);
    return bytes;
}
// my function for reading from pipe
// reads bytes from fd and put them in result
// doing error handling and signaling in case of an error
int myRead(int fd, void* result, int bytes){
    int charsRead = 0;
    
    alarm(30);
    if((charsRead = read(fd, result, bytes)) < 1){
        // printf("I read %d chars\n", charsRead);
        perror(" Error in reading pipe: ");
        kill(getppid(), SIGUSR1);
        exit(NO);
    }
    alarm(0);
    return charsRead;
}

// my function for writing to pipe
// writing toWrite which is bytes
// doing error handling and  everything
int myWrite(int fd, void* toWrite, int bytes){
    int charsWrite = 0;

    alarm(30);
    if ((charsWrite = write(fd, toWrite, bytes)) < 0){
        printf("I wrote %d chars\n", charsWrite);
        perror(" Error in Writing in pipe: ");
        kill(getppid(), SIGUSR2);
        exit(NO);
    }
    alarm(0);
    return charsWrite;
}

// has to write a file into the pipe
void writePipe(int fd, int b, char* actualPath, char* inputDir, char* logfile){
    int isDir = NO;
    char* buffer;
    FILE* logfp;
    int nwrite = 0;
    int bytesWritten = 0;
    int metaWritten = 0;
    short int len = 0;
    unsigned int size = 0;
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
    // write len of file name
    // cut the first part of actual path that is not necessary in backup
    // name of interest to concat for the other process
    pathToBackup = formatBackupPath(inputDir, "", actualPath);
    len = (short int) strlen(pathToBackup);
    // printf("WRITER->Size of name is %s and name %s \n", len, pathToBackup);
    if(isDir == YES){
        //send an identifier l == -1
        short int flag = -1;
        // write the flag
        myWrite(fd, &flag, sizeof(short int));
        // write the name len
        myWrite(fd, &len, sizeof(short int));
        // write name of folder
        myWrite(fd, pathToBackup, len+1);
        free(pathToBackup);
        return;
    }
    // if it is a file
    nwrite = myWrite(fd, &len, sizeof(short int));
    metaWritten += nwrite;
 
    nwrite = myWrite(fd, pathToBackup, len +1);
    metaWritten += nwrite;
    // len of file
    size = (unsigned int) calculateFileSize(actualPath);
    
    nwrite = myWrite(fd, &size, sizeof(unsigned int));
    metaWritten += nwrite;
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
    return;
}

// this function has to read from pipe all the files for this client
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b, char* passPhrase){
    int fd;
    FILE* logfp;
    int nread = 0;
    unsigned int size = 0;
    short int len = 0;
    int bytesRead = 0;
    int metaRead = 0;
    char* filename;
    char* newFile;
    filename = malloc(MAX_PATH_LEN);
    newFile = malloc(MAX_PATH_LEN);
    fd = open(ReceiveData, O_RDONLY);
    if(fd < 0){
        perror(" error in WritePipe: ");
        kill(getppid(), SIGUSR1);
        exit(NO);
    }
    // first read the first two digits, if they are 00, then exit successfully,
    // if they are not continue it is the len of next file or if -1 it is a folder
    while(1){
        strcpy(filename, "");
        strcpy(newFile, "");
        
        nread = myRead(fd, &len, sizeof(short int));
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
            short int folderNameLen = 0;
            folderName = malloc(MAX_PATH_LEN);
            newFolder = malloc(MAX_PATH_LEN);
            strcpy(newFolder, "");
            strcpy(folderName, "");
            myRead(fd, &folderNameLen, sizeof(short int));
            // read folder path

            int read = myRead(fd, folderName, folderNameLen + 1);
            folderName[read] = '\0';
            // make the path to backup
            sprintf(newFolder, "%s/%d/%s", mirrorDir, newID, folderName);
            makeFolder(newFolder);

            continue;
        }

        metaRead += nread;

        nread = myRead(fd, filename, len+1);
        metaRead += nread;
        filename[nread] = '\0';
        nread = myRead(fd, &size, sizeof(unsigned int));
        metaRead += nread;

        // make new file in folder
        // in filename i have the actual path
        if(!strcmp(filename, "")){
            continue;
        }
        sprintf(newFile, "%s/%d/%s", mirrorDir, newID, filename);

        makeFile(newFile);
        // printf("\nREADER->name is %s and size %d \n", newFile, size);

        bytesRead = readFile(fd, size, newFile, b);
        logfp = fopen(logfile, "a");
        // write data in log
        char logdata[200];
        sprintf(logdata, "File Read: %s, Bytes: %d Meta Read: %d", newFile, bytesRead, metaRead);
        fprintf(logfp, "%s\n", logdata);
        fflush(logfp);
        fclose(logfp);
        metaRead = 0;
        if(strcmp(passPhrase, "")){
            // encryption_mode is on!
            // call the decrypt function
            decryptFile(passPhrase, newFile);
        }
    }
    return NO;
}