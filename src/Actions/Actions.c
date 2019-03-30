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
    // printf("I am in here in findFiles!!!!! \n");
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
        // if( isREG(entry->d_name)){
            printf("FileName is %s \n", entry->d_name);
            writePipe(SendData, entry->d_name, b);
        // }
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
    if (write(fd, final, 3) == -1)
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
    if(fp == NULL){
        printf("Unable to open file %s \n", filename);
        exit(EXIT_FAILURE);
    }
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
    char file[b];
    if (read(parentfd, file, b) != b)
    {
        perror(" Error in reading pipe\n");
        exit(2);
    }
    if(write(targetfd, file, b) < 0){
        fprintf(stderr, "Write to file failed.\n");
        exit(EXIT_FAILURE);
    }
    free(file);
}

void writePipe(char* SendData, const char* filename, int b){
    int fd, i, iter, rem;
    unsigned int size;
    unsigned short len;
    // open pipe
    printf("Before %s write end opens \n", SendData);
    fd = open(SendData, O_WRONLY);
    printf("After %s write end opens \n", SendData);
    // write len of file name
    len = strlen(filename);
    printf("WP->Size of name is %hu and name %s \n", len, filename);
    if (write(fd, &len, 2) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    // write filename
    // len + 1 maybe
    if (write(fd, filename, len + 1) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    // len of file
    size = (unsigned int)calculateFileSize(filename);
    printf("---------> size %hu \n", size);
    if (write(fd, &size, 4) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
    size++;
    iter = size / b;
    rem = size % b;

    printf("I calculated size %hu, iter %d, rem %d \n", size, iter, b);

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

void readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b){
    // open pipe
    int fd, i, iter, rem, newFD;
    FILE* logfp;
    long nread = 0;
    unsigned int size = 0;
    unsigned short len = 0;
    char filename[50];
    char newFile[50];
    printf("Before %s read end opens \n", ReceiveData);
    fd = open(ReceiveData, O_RDONLY);
    printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits should be 00
    char* s;
    s = malloc(3);
    strcpy(s,"");
    // if(read(fd, s, 2) == 2 && (!strcmp(s, "00"))){

    // }
        printf("Let's begin reading!\n");

        // if(!strcmp(s, "00")){
        //     printf("I read the 00 bytes from pipe so I m done\n");
        //     break;
        // }
        if((nread = read(fd, &len, 2)) ){
            printf("I read %ld chars --> %d\n", nread, len);
        }
        if((nread = read(fd, filename, len + 1)) ){
            printf("I read %ld chars --> %s\n", nread, filename);
        }
        if((nread = read(fd, &size, 4)) ){
            printf("I read %ld chars ---> %d\n", nread, size);
        }
        // make new file in folder
        sprintf(newFile, "%s/%d/%s", mirrorDir, newID, filename);
        printf("I am %d and i write in Newfile name is %s \n", myID, newFile);
        newFD = open(newFile, O_CREAT | O_APPEND);
        //
        size++;
        iter = size / b;
        rem = size % b;
        for(i=0;i<iter;i++){
            readFile(fd, b, newFD);
            // after i read a part of file open the new file in mirr and append data

        }
        readFile(fd, rem, newFD);
    if((nread = read(fd, s, 3)) ){
        printf("I read %ld chars --> %s\n", nread, filename);
    }
    if(!strcmp(s, "00")){
        printf("I read the 00 bytes from pipe so I m done\n");
    }
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[100];
    sprintf(logdata, "Name: %s, Size: %hu ", filename, size);
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
    // printf("senddata ----> %s \n", SendData);
    // printf("recdata ------> %s \n", ReceiveData);

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
                if(nameExists(SendData) != FILE_){
                    printf("%s desn't exist socreate \n", SendData);
                    mkfifo(SendData, 0666);
                }
                // if (mkfifo(SendData, 0666) == EEXIST)
                // {
                //     printf("%s pipe already exists \n", SendData);
                // }
                // loop over all files of inputDir
                // they are going to be in folders
                // and send their filenames to writepipe
                findFiles(inputDir, SendData, b);

                unlink(SendData);
                exit(1);
            }
            else if (i == 1)
            { // child 2
                printf("I am kid2-----------------> !!! \n");
                if(nameExists(ReceiveData) != FILE_){
                    printf("%s desn't exist so create \n", ReceiveData);
                    mkfifo(ReceiveData, 0666);
                }
                
                printf("fifo made for receiving data !\n");
                // i have the fifo opened to receive data
                readPipe(myID, newID, ReceiveData, mirrorDir, logfile, b);

                unlink(ReceiveData);
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
    // printf("This is the newID function\n");
    // make the corresponding folder in mirrorDir
    struct stat st = {0};
    char newFold[50];
    sprintf(newFold, "%s/%d", mirrorDir, newID);
    // printf("new fold is ------------> %s\n", newFold);
    if (stat(newFold, &st) == -1) {
        mkdir(newFold, 0700);
    }
    spawnKids(commonDir, myID, newID, inputDir, b, mirrorDir, logfile);
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
        if (isDot(d_name)) continue;
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
        exit(EXIT_FAILURE);
    }
}