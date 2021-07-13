#ifndef CLIENT_SERVER_API_H
#define CLIENT_SERVER_API_H

int openConnection(const char* sockname, int msec, const struct timespec abstime);
int closeConnection(const char* sockname);

int openFile(const char* pathname, int flags, const char* dirname);
int readFile(const char* pathname, void** buf, size_t* size);
int readNFiles(int n, const char* dirname);
int writeFile(const char* pathname, void *data, size_t dataSize, const char* dirname);
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);
int lockFile(const char* pathname);
int unlockFile(const char* pathname);
int removeFile(const char* pathname);
int closeFile(const char* pathname);

int readFileFromServer(char **pathname, void **data, int *datLen);

#endif //CLIENT_SERVER_API_H
