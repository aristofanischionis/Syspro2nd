#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/FileOperations.h"
#include "../../HeaderFiles/Encryption.h"

int generateKeys(int myID, char* passPhrase){
    char gentxtName[50];
    pid_t pid;
    FILE* fp;
    strcpy(gentxtName, "");
    sprintf(gentxtName, "encryptionScripts/gen_key_script%d", myID);
    makeFile(gentxtName);
    fp = fopen(gentxtName, "w");
    fprintf(fp, "%%echo Generating a default key for client %d\n", myID);
    fprintf(fp, "Key-Type: DSA\n");
    fprintf(fp, "Key-Length: 1024\n");
    fprintf(fp, "Subkey-Type: ELG-E\n");
    fprintf(fp, "Subkey-Length: 1024\n");
    fprintf(fp, "Name-Real: client%d\n", myID);
    fprintf(fp, "Name-Comment: with genius passphrase\n");
    fprintf(fp, "Name-Email: client%d@syspro.gr\n", myID);
    fprintf(fp, "Expire-Date: 0\n");
    fprintf(fp, "Passphrase: %s\n", passPhrase);
    fprintf(fp, "# Do a commit here, so that we can later print 'done' :-)\n");
    fprintf(fp, "%%commit\n");
    fprintf(fp, "%%echo done\n");
    fclose(fp);
    // i have written everything i needed in this client's gen script
    // it's time to exec the bash script to gen keys for him
    pid = fork(); 
    if(pid == -1){ 
        perror("fork in generateKeys: "); 
        exit(NO); 
    } 
    else if (pid == 0){ 
        char* argvList[] = {"./encryption/gen.sh", gentxtName,NULL}; 
        execv("./encryption/gen.sh", argvList); 
        exit(NO); 
    }
    
    wait(NULL);

    return SUCCESS;
}   

int encryptFile(char* file, char* recepientEmail){
    pid_t pid;
    pid = fork(); 
    if(pid == -1){ 
        perror("fork in encryptFile: "); 
        exit(NO); 
    } 
    else if (pid == 0){ 
        char* argvList[] = {"./encryption/encrypt_file.sh", recepientEmail, file,NULL}; 
        execv("./encryption/encrypt_file.sh", argvList); 
        exit(NO); 
    }
    
    wait(NULL);
    printf("File: %s encrypted Successfully!\n", file);
    fflush(stdout);
    return SUCCESS;
}

int decryptFile(char* passPhrase, char* encryptedFile){
    // make the name decryptedOutput alone
    // the same but with not .asc
    // delete encrypted file from disc
    pid_t pid;
    char* decryptedOutput;
    decryptedOutput = malloc(strlen(encryptedFile) + 1);
    strcpy(decryptedOutput, encryptedFile);
    char *dotPos;
    dotPos = strrchr(decryptedOutput, '.');
    strcpy(dotPos, "");
    // printf("=---------------------=decrypted filename: %s \n", decryptedOutput);
    pid = fork(); 
    if(pid == -1){ 
        perror("fork in decryptFile: "); 
        exit(NO); 
    } 
    else if (pid == 0){ 
        char* argvList[] = {"./encryption/decrypt_file.sh", passPhrase, encryptedFile, decryptedOutput, NULL}; 
        execv("./encryption/decrypt_file.sh", argvList); 
        exit(NO); 
    }
    
    wait(NULL);
    printf("File: %s decrypted Successfully!\n", decryptedOutput);
    fflush(stdout);
    unlink(encryptedFile);
    return SUCCESS;
}

int findEmail(char* commonDir, int recepientID, char* recepientEmail){
    DIR *dir;
    pid_t pid;
    char result[40];
    FILE* fp;
    struct dirent *entry;
    char path[MAX_PATH_LEN];

    if ((dir = opendir(commonDir)) == NULL) perror("opendir() error");
    else {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.'){
                strcpy(path, commonDir);
                strcat(path, "/");
                strcat(path, entry->d_name);
                char *dot = strrchr(path, '.');
                if (dot && !strcmp(dot, ".id")){
                    // I found a filename that ends with .id
                    int thisID = 0;
                    sscanf(entry->d_name, "%d.id", &thisID);
                    if(thisID == recepientID){
                        fp = fopen(path, "r");
                        if(fp == NULL){
                            fprintf(stderr, "Could not open '%s': %s\n", path, strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        fscanf(fp, "%d %s", &pid, result);
                        fclose(fp);
                        strcpy(recepientEmail, result);
                        if(!strcmp(recepientEmail, "")){
                            return ERROR;
                        }
                        printf("I found the email: %s\n", recepientEmail);
                        closedir(dir);
                        return SUCCESS;
                    }
                }        
            }
        }
    }
    return ERROR;
}


