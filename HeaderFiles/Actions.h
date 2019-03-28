#ifndef ACTIONS_CODE
#define ACTIONS_CODE
#define FIFO_LEN 50

// #define isDIR(X) ((X) == DT_DIR ? (1) : (0))
// #define isREG(X) ((X) == DT_REG ? (1) : (0))

int newID(const char* commonDir, const char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile);
int removeID();

#endif