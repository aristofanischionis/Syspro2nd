#ifndef ACTIONS_CODE
#define ACTIONS_CODE
#define FIFO_LEN 50
#define PIPE_OP 7
#define DEAD 123
#define ALIVE 321
#define ON 1234
#define OFF 4321

void syncr(int myID, char *commonDir, int b, char* inputDir, char* mirrorDir, char* logfile, char* passPhrase);
int newID(char* commonDir, char* inputDir, int myID, int newID, int b, char* mirrorDir, char* logfile, char* passPhrase);
int removeID(char* inputDir, int deletedID, char* mirrorDir);
void handle_alarm();

#endif