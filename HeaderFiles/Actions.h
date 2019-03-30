#ifndef ACTIONS_CODE
#define ACTIONS_CODE
#define FIFO_LEN 50

void syncr(int myID, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile);
int newID(const char* commonDir, const char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile);
int removeID();

#endif