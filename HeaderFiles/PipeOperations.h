#ifndef PIPE_HEADER
#define PIPE_HEADER

void writeFinal(int fd);
void writeFile(char* filename, int parentfd, int size, int b);
void readFile(int parentfd, int size, char* newFile, int b);
void writePipe(char* SendData, int b, char* actualPath, char* inputDir);
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b);

#endif