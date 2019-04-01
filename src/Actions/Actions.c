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
#include <signal.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"
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

void writePipe(char* SendData, int b, char* actualPath, char* inputDir);

// Make an identical path to sourcePath, but with backupBase as the root
char *formatBackupPath(char *sourceBase, char *backupBase, char *sourcePath) {
    char *backupPath = malloc(300);
    strcpy(backupPath, backupBase);
    char sourcePathCopy[300];
    strcpy(sourcePathCopy, sourcePath);
    size_t len = strlen(sourceBase);
    if (len > 0) {
        char *p = sourcePathCopy;
        while ((p = strstr(p, sourceBase)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    strcat(backupPath, sourcePathCopy);
    if (backupPath[0] == '/'){
        memmove(backupPath, backupPath+1, strlen(backupPath));
    }
    
    if (backupPath[strlen(backupPath) - 1] == '/') {
        backupPath[strlen(backupPath) - 1] = 0;
    }
    return backupPath;
}

// int isREG(char *path)
// {
//     struct stat path_stat;
//     stat(path, &path_stat);
//     return S_ISREG(path_stat.st_mode);
// }

void findFiles(char *source, int indent, char* SendData, int b, char* inputDir) {
    DIR *dir;
    struct dirent *entry;
    char path[1025];
    struct stat info;

    if ((dir = opendir(source)) == NULL)
        perror("opendir() error");
    else {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                strcpy(path, source);
                strcat(path, "/");
                strcat(path, entry->d_name);
                if (stat(path, &info) != 0){
                    fprintf(stderr, "stat() error on %s: %s\n", path, strerror(errno));
                }
                else if (S_ISDIR(info.st_mode)){
                    findFiles(path, indent+1, SendData, b, inputDir);
                }
                else {
                    writePipe(SendData, b, path, inputDir);
                }
            }
        }
        closedir(dir);
    }
}

void writeFinal(int fd)
{   
    char final[3] = "00";
    if (write(fd, final, 3) <0)
    {
        perror(" Error in Writing in pipe\n");
        kill(parentPid, SIGUSR2);
        exit(NO);
    }
}

long calculateFileSize(char* filename){
    FILE* fp;    
    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Unable to open file %s \n", filename);
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    fclose(fp);
    return size;
}

void writeFile(char* filename, int parentfd, int b){
    FILE* fp;
    char *file;
    file = malloc(b);
    strcpy(file, "");
    printf("the file ---> %s \n", filename);
    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Unable to open file %s \n", filename);
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // read 
    if(fread(file, 1, b, fp) < 0){
        fprintf(stderr, "read from file failed.\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    printf("write file read : %s \n", file);
    if((write(parentfd, file, b)) < 0){
        fprintf(stderr, "Write to pipe failed.\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    free(file);      
    fclose(fp);
}

void readFile(int parentfd, int b, char* newFile){
    FILE* newFD = fopen(newFile, "a");
    if (newFD == NULL) {
        perror("Failed------------>: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    char *file;
    file = malloc(b);
    strcpy(file, "");
    if (read(parentfd, file, b) < 0)
    {
        perror(" Error in reading pipe\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // printf("readFile has read : %s bytes %d , file %s\n", file, b, newFile);
    if(fprintf(newFD, "%s", file) < 0){
        perror("write to newfile failed\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    free(file);
    fclose(newFD);
}

void writePipe(char* SendData, int b, char* actualPath, char* inputDir){
    int fd, i, iter = 0, rem = 0;
    unsigned int size;
    unsigned short len;
    char* pathToBackup;
    pathToBackup = malloc(300);
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
    if (write(fd, &len, 2) < 0)
    {
        perror(" Error in Writing in pipe1\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // write 
    // len + 1 maybe
    if (write(fd, pathToBackup, len + 1) < 0)
    {
        perror(" Error in Writing in pipe2\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    // len of file
    size = (unsigned int)calculateFileSize(actualPath);

    if (write(fd, &size, 4) < 0)
    {
        perror(" Error in Writing in pipe3\n");
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
    writeFinal(fd);
    close(fd);
}

int makeFile(char* filename){
    // find path and file
    char copy[300];
    char file[50];
    char tmp[300];
    char path[250];
    strcpy(path, "");
    strcpy(copy, filename);
    const char s[2] = "/";
    char *token;
    token = strtok(copy, s);

    while( token != NULL ) {
        strcpy(tmp, token);
        token = strtok(NULL, s);
        if(token == NULL){
            // just found the file
            strcpy(file, tmp);
        }
        else {
            sprintf(path, "%s/%s", path, tmp);
        }
    }
    if (path[0] == '/'){
        memmove(path, path+1, strlen(path));
    }
    printf("filename is %s \n", filename);
    printf("My path is %s \n", path);
    printf("My file is %s \n", file);
    // make any subdirectories first using mkdir -p
    pid_t pid, wpid;
    int status = 0;
    pid = fork();
    if (pid == 0) {
        execl("/bin/mkdir", "mkdir", "-p", path, NULL);
    } else if (pid < 0) {
        perror("pid<0 in mkdir\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    while ((wpid = wait(&status)) > 0); 
    
    // then use touch
    pid = fork();
    status = 0;
    if (pid == 0) {
        execl("/bin/touch", "touch", filename, NULL);
    } else if (pid < 0) {
        perror("pid<0 in touch\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    while ((wpid = wait(&status)) > 0);
    
    return YES;
}

int deleteFolder(char* folder){
    pid_t pid, wpid;
    int status = 0;
    pid = fork();
    if (pid == 0) {
        execl("/bin/rm", "rm", "-rf", folder, NULL);
    } else if (pid < 0) {
        perror("pid<0 in rm -rf\n");
        return ERROR;
    }
    while ((wpid = wait(&status)) > 0); 
    return SUCCESS;
}

// readpipe needs the input dir base from the other process
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b){
    // open pipe
    int fd, i, iter, rem = 0;
    int flag = 0;
    FILE* logfp;
    long nread = 0;
    unsigned int size = 0;
    unsigned short len = 0;
    char filename[300];
    char newFile[300];
    printf("Before %s read end opens \n", ReceiveData);
    fd = open(ReceiveData, O_RDONLY);
    printf("After %s read end opens \n", ReceiveData);
    // first read the first two digits should be 00
    char s[3];
    
    if((nread = read(fd, &len, 2)) < 0){
        printf("I read %ld chars\n", nread);
    }
    if((nread = read(fd, filename, len + 1)) < 0){
        printf("I read %ld chars\n", nread);
    }
    if((nread = read(fd, &size, 4)) < 0){
        printf("I read %ld chars\n", nread);
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
    if((nread = read(fd, s, 3)) < 0){
        printf("i read %ld chars\n", nread);
    }

    if(!strcmp(s, "00")){
        // printf("I read the 00 bytes from pipe so I m done\n");
        // successful
        flag = 1;
    }
    logfp = fopen(logfile, "a");
    // write data in log
    char logdata[100];
    sprintf(logdata, "Name: %s, Size: %hu ", filename, size);
    printf(" ----------> to be written in log %s \n", logdata);
    fprintf(logfp, "%s\n", logdata);
    fclose(logfp);
    close(fd);
    return (flag == 1) ? YES : NO;
}

int spawnKids(char* commonDir, int myID, int newID, char* inputDir, int b, char* mirrorDir, char* logfile){
    // if(doAgain == NO){
    //     return ERROR;
    // }
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
            { // child 1
                printf("I am kid1------------------> !!! \n");
                if(nameExists(SendData) != FILE_){
                    printf("%s desn't exist socreate \n", SendData);
                    mkfifo(SendData, 0666);
                }
                findFiles(inputDir, 0, SendData, b, inputDir);
                unlink(SendData);
                exit(YES);
            }
            else if (i == 1)
            { // child 2
                printf("I am kid2-----------------> !!! \n");
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
    // printf("This is the newID function\n");
    // make the corresponding folder in mirrorDir
    int retv;
    struct stat st = {0};
    char newFold[50];
    sprintf(newFold, "%s/%d", mirrorDir, newID);
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