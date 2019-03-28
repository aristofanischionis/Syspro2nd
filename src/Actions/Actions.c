#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"
void writePipe(char* SendData, const char* filename, int b);


int isDot(char *name) {
    return (((!strcmp(name, ".")) || (!strcmp(name, ".."))) ? (1) : (0));
}

int isREG(char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

// Opens directories and do stuff with files
void findFiles(const char *source, char* SendData, int b) {
    DIR *d;
    /* Open the directory specified by "source". */
    d = opendir(source);
    /* Check it was opened. */
    if (!d) {
        printf("Cannot open directory source in findFiles\n");
        return;
    }
    printf("I am in here in findFiles!!!!! \n");
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
        if (isDot(d_name)) continue;
        // if it is a file do stuff
        char buf[MAX_PATH_LEN];
        if (isREG(realpath(entry->d_name, buf))) {
            // entry->d_name is the name
            printf("FileName is %s \n", entry->d_name);
            // write it to pipe
            writePipe(SendData, entry->d_name, b);
        }
        
    }
    /* After going through all the entries, close the directory. */
    if (closedir(d)) {
        fprintf(stderr, "Could not close '%s': %s\n", source, strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void writeFinal(int fd)
{   
    char final[3] = "00";
    if (write(fd, &final, sizeof(final)) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
}

long calculateFileSize(const char* filename){
    FILE* fp;
    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Unable to open file %s \n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    fclose(fp);
    return size;
}

void writeFile(const char* filename, int parentfd, int b){
    FILE* fp;
    char c;
    int counter = 0;
    fp = fopen(filename, "r");
    while(counter < b){
        c = fgetc(fp);
        if((write(parentfd, &c, 1)) < 1){
            fprintf(stderr, "Write to pipe failed.\n");
            exit(EXIT_FAILURE);
        }
        counter++;
    }
    fclose(fp);
}

void readFile(int parentfd, int b, int targetfd){
    // fp = open(filename, O_APPEND);
    char* file;
    file = malloc(b * sizeof(char));
    strcpy(file, "");

    if (read(parentfd, &file, b) != b)
    {
        perror(" Error in reading pipe\n");
        exit(2);
    }
    if(write(targetfd, &file, b) < 0){
        fprintf(stderr, "Write to file failed.\n");
        exit(EXIT_FAILURE);
    }
    free(file);
}

void writePipe(char* SendData, const char* filename, int b){
    int fd, i, iter, rem;
    long size;
    size_t len;
    // open pipe
    if ((fd = open(SendData, O_WRONLY)) < 0)
    {
        perror("fifo open error");
        exit(1);
    }
    // write len of file name
    len = strlen(filename);
    printf("Size of name is %ld and name %s \n", len, filename);
    if (write(fd, &len, 2) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    // write filename
    // len + 1 maybe
    if (write(fd, &filename, len) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    // len of file
    size = calculateFileSize(filename);
    if (write(fd, &size, 4) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    iter = size / b;
    rem = size % b;

    printf("I calculated size %ld, iter %d, rem %d \n", size, iter, b);

    for(i=0;i<iter;i++){
        // write part of the file data
        writeFile(filename, fd, b);
    }
    writeFile(filename, fd, rem);
    // wrote all of the file in fd
    // actual file in chunks size of b
    writeFinal(fd);
    close(fd);
}

void readPipe(int myID, char* ReceiveData, char* mirrorDir, char* logfile, int b){
    // open pipe
    int fd, i, iter, rem, newFD;
    FILE* logfp;
    long nread, size;
    size_t len;
    char* filename;
    char* newFile;
    newFile = malloc(50);
    filename = malloc(50);
    strcpy(filename, "");
    strcpy(newFile, "");

    if ((fd = open(ReceiveData, O_RDONLY)) < 0)
    {
        perror("fifo open error");
        exit(1);
    }
    if((nread = read(fd, &len, 2)) !=2 ){
        printf("I read %ld chars\n", nread);
    }
    if((nread = read(fd, &filename, len)) != len ){
        printf("I read %ld chars\n", nread);
    }
    if((nread = read(fd, &size, 4)) != 4 ){
        printf("I read %ld chars\n", nread);
    }
    // make new file in folder
    sprintf(newFile, "%s/%d/%s", mirrorDir, myID, filename);
    printf("Newfile name is %s \n", newFile);
    newFD = open(newFile, O_CREAT | O_APPEND);
    logfp = fopen(logfile, "a");
    //
    iter = size / b;
    rem = size % b;
    for(i=0;i<iter;i++){
        readFile(fd, b, newFD);
        // after i read a part of file open the new file in mirr and append data

    }
    readFile(fd, rem, newFD);
    // write data in log
    char logdata[100];
    sprintf(logdata, "Name: %s, Size: %ld ", filename, size);
    printf(" ----------> to be written in log %s \n", logdata);
    fwrite(logdata, 1, 100, logfp);
    
    fclose(logfp);
    close(newFD);
    close(fd);
}

void spawnKids(const char* commonDir, int myID, int newID, const char* inputDir, int b, char* mirrorDir, char* logfile){
    char SendData[FIFO_LEN];
    char ReceiveData[FIFO_LEN];
    char* filename;
    filename = malloc(50);
    strcpy(filename, "");
    // giving name to each fifo

    sprintf(SendData, "%s/%d_to_%d.fifo", commonDir, myID, newID);
    sprintf(ReceiveData, "%s/%d_to_%d.fifo", commonDir, newID, myID);
    //
    printf("senddata ----> %s \n", SendData);
    printf("recdata ------> %s \n", ReceiveData);

    // forking the kids
    pid_t pid, wpid;
    for (int i = 0; i < 2; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            // do childern stuff
            if (i == 0)
            { // child 1
                printf("I am kid1------------------> !!! \n");
                if (mkfifo(SendData, 0666) == EEXIST)
                {
                    printf("%s pipe already exists \n", SendData);
                }
                // open pipe
                printf("fifo made!");
                // i have the fifo opened to send data
                // loop over all files of inputDir
                // they are going to be in folders
                // and send their filenames to writepipe
                printf("Right before findfileessssssssss\n");
                findFiles(inputDir, SendData, b);

                remove(SendData);
                exit(1);
            }
            else if (i == 1)
            { // child 2
                printf("I am kid2-----------------> !!! \n");
                if (mkfifo(ReceiveData, 0666) == EEXIST)
                {
                    printf("%s pipe already exists \n", ReceiveData);
                }
                printf("fifo made for receiving data !");
                // i have the fifo opened to receive data
                readPipe(myID, ReceiveData, mirrorDir, logfile, b);

                remove(ReceiveData);
                exit(0);
            }
        }
    }
    // parent
    int status = 0;
    while ((wpid = wait(&status)) > 0)
    {
        printf("Exit status of %d was %d (%s)\n", (int)wpid, status,
               (status > 0) ? "kid1" : "kid2");
    }
    
}


int newID(const char* commonDir , const char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile){
    printf("This is the newID function\n");

    spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile);
    return SUCCESS;
}

int removeID(){
    printf("This is the removeID function\n");
    return SUCCESS;
}