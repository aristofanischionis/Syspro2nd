#ifndef FILE_HEADER
#define FILE_HEADER

char* formatBackupPath(char* sourceBase, char* backupBase, char* sourcePath);
void findFiles(char *source, int indent, char* SendData, int b, char* inputDir);
long calculateFileSize(char* filename);
int makeFile(char* filename);
int deleteFolder(char* folder);

#endif