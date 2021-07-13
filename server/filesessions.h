#ifndef PROGETTO_MICHELE_FILESESSIONS_H
#define PROGETTO_MICHELE_FILESESSIONS_H

typedef struct {
    int connfd;
    char *pathname;
    int flags;
} sessionDescriptor_t;


void initializeSessionsDescritors(int maxFiles, int maxSize);
void deinitializeSessionsDescritors();

int getMaxFiles();

int openFile(int connfd, char *pathname, int flags, char * dirname);
int closeFile(int connfd, char *pathname);
int lockFile(int connfd, char *pathname);
int unlockFile(int connfd, char *pathname);
int removeFile(int connfd, char *pathname);
int readFileData(int connfd,char *pathname, void** data, int*sizeOfData);
int readNextFile(int idx, void **data, int* dataLen, char **pathname);
int writeFile(int connfd, char *pathname, void *buffer, int sizeOfData, char * dirname);
int appendaFileData(int connfd, char *pathname, void *buffer, int sizeOfData ,char *dirname);

#define  UNKNOWN_ERROR              (-1)
#define  FILE_ALREADY_OPEN          (-2)
#define  FILE_ALREADY_LOCKED        (-3)
#define  FILE_CREATE_NOT_POSSIBLE   (-4)
#define  FILE_NOT_FOUND             (-5)
#define  FILE_NEVER_OPEN            (-6)
#define  FILE_NEVER_LOCKED          (-7)

#endif //PROGETTO_MICHELE_FILESESSIONS_H
