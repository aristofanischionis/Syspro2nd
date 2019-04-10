#ifndef PIPE_HEADER
#define PIPE_HEADER

void writeFinal(int fd);
int writeFile(char* filename, int parentfd, int size, int b);
int readFile(int parentfd, int size, char* newFile, int b);
void writePipe(char* SendData, int b, char* actualPath, char* inputDir, char* logfile);
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b, char* passPhrase);

#endif