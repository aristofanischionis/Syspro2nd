#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"

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
void findFiles(const char *source, int fd, int b) {
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
            // 
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

void writePipe(int fd, const char* filename, int b){

    // write len of file name
    size_t len = strlen(filename);
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

    // actual file in chunks size of b
    writeFinal(fd);
}

void readPipe(int fd){

}

void spawnKids(const char* commonDir, int myID, int newID, const char* inputDir, int b){
    int fd1, fd2;
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
    pid_t pid;
    for (int i = 0; i < 2; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            // do childern stuff
            if (i == 0)
            { // child 1
                printf("I am kid1 !!! \n");
                if (mkfifo(SendData, 0666) == EEXIST)
                {
                    printf("%s pipe already exists \n", SendData);
                }
                // open pipe
                printf("fifo made !");
                if ((fd1 = open(SendData, O_WRONLY)) < 0)
                {
                    perror("fifo open error");
                    exit(1);
                }
                // i have the fifo opened to send data
                // loop over all files of inputDir
                // they are going to be in folders
                // and send their filenames to writepipe
                printf("Right before findfileessssssssss\n");
                findFiles(inputDir, fd1, b);

                close(fd1);
                remove(SendData);
                exit(SUCCESS);
            }
            else
            { // child 2
                printf("I am kid2 !!! \n");
                if (mkfifo(ReceiveData, 0666) == EEXIST)
                {
                    printf("%s pipe already exists \n", SendData);
                }
                // open pipe
                if ((fd2 = open(ReceiveData, O_RDONLY)) < 0)
                {
                    perror("fifo open error");
                    exit(1);
                }
                // i have the fifo opened to receive data
                readPipe(fd2);

                close(fd2);
                remove(ReceiveData);
                exit(SUCCESS);
            }
        }
        else if (pid)
        {
            continue;
        }
        else
        {
            printf("Error in forking kids\n");
            exit(1);
        }
    }
    // parent
    // printf(" I am the parent process % d\n", getpid());

    // if ((fd1 = open(SendData, O_RDONLY)) < 0)
    // {
    //     perror("fifo open error");
    //     exit(1);
    // }
    // if ((fd2 = open(ReceiveData, O_RDONLY)) < 0)
    // {
    //     perror("fifo open error");
    //     exit(1);
    // }
    // readWritefifos(fd2, myfd);
    // readWritefifos(fd1, myfd);

    // writeFakeRecord(myfd);
    
}


int newID(const char* commonDir , const char* inputDir, int myID, int newID, int b){
    printf("This is the newID function\n");

    spawnKids(commonDir, myID, newID, inputDir, b);
    return SUCCESS;
}

int removeID(){
    printf("This is the removeID function\n");
    return SUCCESS;
}