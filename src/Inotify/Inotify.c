#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/types.h>
#include "../../HeaderFiles/Inotify.h"
#include "../../HeaderFiles/Actions.h"
#include "../../HeaderFiles/Input.h"

// standard code got from the man page
/* Read all available inotify events from the file descriptor 'fd'.
          wd is the table of watch descriptors for the directories in argv.
          argc is the length of wd and argv.
          argv is the list of watched directories.
          Entry 0 of wd and argv is unused. */

static void handle_events(int myID, int fd, int wd, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile, char* passPhrase)
{
    /* Some systems cannot read integer variables if they are not
              properly aligned. On other systems, incorrect alignment may
              decrease performance. Hence, the buffer used for reading from
              the inotify file descriptor should have the same alignment as
              struct inotify_event. */

    char buf[4096]
        __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;
    char *ptr;

    /* Loop while events can be read from inotify file descriptor. */
    for (;;)
    {
        /* Read some events. */
        len = read(fd, buf, sizeof buf);
        if (len == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* If the nonblocking read() found no events to read, then
                  it returns -1 with errno set to EAGAIN. In that case,
                  we exit the loop. */
        if (len <= 0)
            break;
        /* Loop over all events in the buffer */

        for (ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event *)ptr;

            if(event->len){
                char *dot = strrchr(event->name, '.');
                if (dot && !strcmp(dot, ".id")){
                    // I found a filename that ends with .id
                    int thisID = 0;
                    sscanf(event->name, "%d.id", &thisID);
                    printf("I found an event for id = %d \n", thisID);
                    if (event->mask & IN_CREATE){
                        // fork a kid so don't lose time on waiting here till it finishes
                        pid_t pid;
                        pid = fork();
                        if (pid == 0) {
                            if(newID(commonDir, inputDir, myID, thisID, b, mirrorDir, logfile, passPhrase) == ERROR){
                                printf("NewID function returned an error \n");
                                exit(EXIT_FAILURE);
                            }
                            exit(YES);
                        }
                    }
                    else if (event->mask & IN_DELETE){
                        if(removeID(inputDir, thisID, mirrorDir) == ERROR){
                            printf("removeID function returned an error \n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else {
                        printf("FileName %s doesn't have an apropriate mask", event->name);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}

int inotifyCode(int myID, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile, char* passPhrase)
{
    int fd;
    int wd;
    /* Create the file descriptor for accessing the inotify API */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
    {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, commonDir, IN_CREATE | IN_DELETE);

    if (wd == -1)
    {
        fprintf(stderr, "Cannot watch '%s'\n", commonDir);
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    printf("Listening for events.\n");
    while(1){
        handle_events(myID, fd, wd, commonDir, b, inputDir, mirrorDir, logfile, passPhrase);
    }
    
    printf("Listening for events stopped.\n");
    /* Close inotify file descriptor */
    close(fd);
    exit(EXIT_SUCCESS);
}