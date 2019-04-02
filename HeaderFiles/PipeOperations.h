#ifndef PIPE_HEADER
#define PIPE_HEADER

void writeFinal(int fd);
void writeFile(char* filename, int parentfd, int b);
void readFile(int parentfd, int b, char* newFile);
void writePipe(char* SendData, int b, char* actualPath, char* inputDir);
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b);

#endif