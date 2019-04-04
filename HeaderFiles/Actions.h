#ifndef ACTIONS_CODE
#define ACTIONS_CODE
#define FIFO_LEN 50
#define PIPE_OP 7

void syncr(int myID, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile);
int newID(char* commonDir, char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile);
int removeID(char* inputDir, int deletedID, char* mirrorDir);

#endif