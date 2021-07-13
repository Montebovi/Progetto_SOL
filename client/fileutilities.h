#ifndef CLIENT_FILEUTILITIES_H
#define CLIENT_FILEUTILITIES_H

#include <stdbool.h>

int getNumOfFilesOfDirectoryRec(char * name);
void getFilesOfDirectoryRec(const char *name, char** fNames, int *idx, int maxFiles);
bool directoryExists(char * name);
int saveFile(char * dirname, char *pathname, void *data, int dataLen);
int loadFile(char *pathname, void** buffer, size_t*dataLen);

#endif //CLIENT_FILEUTILITIES_H
