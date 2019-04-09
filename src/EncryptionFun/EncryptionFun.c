#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../../HeaderFiles/Input.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/FileOperations.h"
#include "../../HeaderFiles/Encryption.h"

int generateKeys(int myID){
    char gentxtName[50];
    pid_t pid, wpid;
    int status = 0;
    FILE* fp;
    strcpy(gentxtName, "");
    sprintf(gentxtName, "encryptionScripts/gen_key_script%d", myID);
    makeFile(gentxtName);
    fp = fopen(gentxtName, "w");
    fprintf(fp, "%%echo Generating a default key for client %d\n", myID);
    fprintf(fp, "Key-Type: default\n");
    fprintf(fp, "Subkey-Type: default\n");
    fprintf(fp, "Name-Real: client%d\n", myID);
    fprintf(fp, "Name-Comment: with genius passphrase\n");
    fprintf(fp, "Name-Email: client%d@syspro.gr\n", myID);
    fprintf(fp, "Expire-Date: 0\n");
    fprintf(fp, "Passphrase: client%d\n", myID);
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
    
    while ((wpid = wait(&status)) > 0);

    return SUCCESS;
}   
