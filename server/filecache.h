#ifndef PROGETTO_MICHELE_FILECACHE_H
#define PROGETTO_MICHELE_FILECACHE_H

#include <stdbool.h>

int store_initialize(int maxFiles, int maxSize);
void store_deinitialize();

int store_maxFiles();
int store_createFile(char *pathname, bool isLocked,char *dirname);
int store_removeFile(char *pathname);
int store_lockFile(char *pathname);
int store_unlockFile(char *pathname);
int store_readFile(char* pathname , void** data, int*sizeOfData);
int store_writeFile(char* pathname, char *dirname, void *buffer, size_t len, bool appendMode);
int store_readNextFile(int idx, void **data, int* dataLen, char **pathname);

bool store_isLocked(char* pathname);
bool store_isPresent(char* pathname);

void printStatus();

#endif //PROGETTO_MICHELE_FILECACHE_H
