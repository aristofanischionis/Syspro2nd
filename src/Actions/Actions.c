#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"

void writeFinal(int fd)
{   
    char final[3] = "00";
    if (write(fd, &final, sizeof(final)) == -1)
    {
        perror(" Error in Writing in pipe\n");
        exit(2);
    }
}

void writePipe(int fd, const char* filename){

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

    // actual file
    writeFinal(fd);
}

void readPipe(int fd){

}

void spawnKids(const char* commonDir, int myID, int newID, const char* inputDir){
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
    // printf("senddata ----> %s \n", SendData);
    // printf("recdata ------> %s \n", ReceiveData);

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
                if (mkfifo(SendData, 0666) == EEXIST)
                {
                    printf("%s pipe already exists \n", SendData);
                }
                // open pipe
                if ((fd1 = open(SendData, O_WRONLY)) < 0)
                {
                    perror("fifo open error");
                    exit(1);
                }
                // i have the fifo opened to send data
                // loop over all files of inputDir
                // and send their filenames to writepipe
                
                writePipe(fd1, filename);

                close(fd1);
                remove(SendData);
                // exit(SUCCESS);
            }
            else
            { // child 2
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
                // exit(SUCCESS);
            }
        }
        else if (pid == -1)
        {
            printf("Error in forking kids\n");
            exit(ERROR);
        }
        else
        {
            continue;
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


int newID(const char* commonDir , int myID, int newID){
    printf("This is the newID function\n");

    spawnKids(commonDir, myID, newID);
    return SUCCESS;
}

int removeID(){
    printf("This is the removeID function\n");
    return SUCCESS;
}