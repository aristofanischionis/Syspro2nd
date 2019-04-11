#ifndef ENCRYPTION_CODE
#define ENCRYPTION_CODE

int generateKeys(int myID, char* passPhrase);
int findEmail(char* commonDir, int recepientID, char* recepientEmail);
int decryptFile(char* passPhrase, char* encryptedFile);
int encryptFile(char* file, char* recepientEmail);

#endif