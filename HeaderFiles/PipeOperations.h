#ifndef PIPE_HEADER
#define PIPE_HEADER

void writeFinal(int fd);
int writeFile(char* filename, int parentfd, int size, int b);
int readFile(int parentfd, int size, char* newFile, int b);
void writePipe(int fd, int b, char* actualPath, char* inputDir, char* logfile);
int readPipe(int myID, int newID, char* ReceiveData, char* mirrorDir, char* logfile, int b, char* passPhrase);
int myRead(int fd, void* result, int bytes);
int myWrite(int fd, void* toWrite, int bytes);
void handle_SIGPIPE();

#endif