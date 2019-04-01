#ifndef INPUT_HEADER
#define INPUT_HEADER
#define MAX_PATH_LEN 100
#define SUCCESS 0
#define ERROR 1
#define YES 2
#define NO 3
#define FILE_ 4
#define DIR_ 5
#define FIFO 6

int InputReader(int argc, char *argv[]);
int nameExists(char* filename);
#endif