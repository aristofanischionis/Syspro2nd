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
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/PipeOperations.h"
#include "../../HeaderFiles/FileOperations.h"
#include "../../HeaderFiles/Encryption.h"
#include <signal.h>
pid_t parentPid;

// Make an identical path to sourcePath, but with backupBase as the root
char* formatBackupPath(char* sourceBase, char* backupBase, char* sourcePath) {
    char *backupPath = malloc(MAX_PATH_LEN);
    strcpy(backupPath, backupBase);
    char sourcePathCopy[MAX_PATH_LEN];
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

void findFiles(char *source, int indent, char* SendData, int b, char* inputDir, char* logfile, char* recepientEmail){
    parentPid = getppid();
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH_LEN];
    struct stat info;

    if ((dir = opendir(source)) == NULL) perror("opendir() error");
    else {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.'){
                strcpy(path, source);
                strcat(path, "/");
                strcat(path, entry->d_name);
                if (stat(path, &info) != 0){
                    fprintf(stderr, "stat() error on %s: %s\n", path, strerror(errno));
                }
                else if (S_ISDIR(info.st_mode)){
                    writePipe(SendData, b, path, inputDir, logfile);
                    findFiles(path, indent+1, SendData, b, inputDir, logfile, recepientEmail);
                }
                else {
                    // it is a file
                    // so encrypt it
                    encryptFile(path, recepientEmail);
                    // make the path.asc file in char* to pass to writePipe
                    char* encr;
                    encr = malloc(MAX_PATH_LEN);
                    strcpy(encr, "");
                    sprintf(encr, "%s.asc", path);
                    // write it to pipe
                    writePipe(SendData, b, encr, inputDir, logfile);
                    // delete the encrypted copy in input_dir
                    unlink(encr);
                }
            }
        }
        closedir(dir);
    }
}

long int calculateFileSize(char* filename){
    FILE* fp;    
    fp = fopen(filename, "r");
    if(fp == NULL){
        perror("Unable to open file: ");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }
    fseek(fp, 0L, SEEK_END);
    long int size = ftell(fp);
    rewind(fp);
    fclose(fp);
    return size;
}

void makeFolder(char* foldername){
    if(foldername && !foldername[0]){
        printf("foldername is empty \n");
        return;
    }
    // printf("Folder to be made %s\n", foldername);
    // make any subdirectories first using mkdir -p
    pid_t pid, wpid;
    int status = 0;
    pid = fork();
    if (pid == 0) {
        execl("/bin/mkdir", "mkdir", "-p", foldername, NULL);
    } 
    else if(pid < 0) {
        perror("pid<0 in mkdir\n");
        kill(parentPid,SIGUSR2);
        exit(NO);
    }

    while ((wpid = wait(&status)) > 0); 
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
    // printf("filename is %s \n", filename);
    // printf("My path is %s \n", path);
    // printf("My file is %s \n", file);
    // make any subdirectories first using mkdir -p
    pid_t pid, wpid;
    int status = 0;
    
    makeFolder(path); 
    
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
    if(folder && !folder[0]){
        printf("folder is empty in deleteFolder \n");
        return ERROR;
    }
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
