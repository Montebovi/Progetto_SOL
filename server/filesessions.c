#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/svc_commands.h"
#include "filecache.h"
#include "filesessions.h"
#include "util.h"

typedef struct {
    sessionDescriptor_t **array;
    size_t used;
    size_t size;
} Array;

static  pthread_mutex_t _filesessions_mutex;

static  Array *descriptors;

#define INITIALSIZE  (2)

//---------------------------------------------------------------------------------------
static void freeDescriptor(int idx){
    if (idx == -1)
        return;
    sessionDescriptor_t * sessionDescr = descriptors->array[idx];
    if (sessionDescr){
        if (sessionDescr->pathname){
            free(sessionDescr->pathname);
            sessionDescr->pathname = NULL;
        }
        free(sessionDescr);
        sessionDescr = NULL;
    }
    descriptors->array[idx] = 0;
}

//---------------------------------------------------------------------------------------
static int findSessionIndex(int connfd, char *pathname){
    for(int i=0; i<descriptors->size; i++){
        if (descriptors->array[i] != 0 &&
        descriptors->array[i]->connfd == connfd && (strcmp(descriptors->array[i]->pathname, pathname)==0))
            return i;
    }
    return -1;
}

//---------------------------------------------------------------------------------------
static sessionDescriptor_t* findSession(int connfd, char *pathname){
    for(int i=0; i<descriptors->size; i++){
        if (descriptors->array[i] != 0
        && descriptors->array[i]->connfd == connfd
        && (strcmp(descriptors->array[i]->pathname, pathname)==0))
            return descriptors->array[i];
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
static  sessionDescriptor_t* createSession(int connfd, char *pathname,int flags){
    sessionDescriptor_t* nuovaSessione = malloc(sizeof(sessionDescriptor_t));
    nuovaSessione->connfd = connfd;
    nuovaSessione->pathname = strdup(pathname);
    nuovaSessione->flags = flags;

    if (descriptors->used == descriptors->size) {  // tutti usati
        descriptors->size *= 2;
        descriptors->array = realloc(descriptors->array, descriptors->size * sizeof(sessionDescriptor_t *));
        for (int i=descriptors->used; i<descriptors->size;i++)
            descriptors->array[i]=0;
        descriptors->array[descriptors->used] = nuovaSessione;
    }
    else{
        for (int i=0; i<descriptors->size;i++)
            if (descriptors->array[i] == 0){
                descriptors->array[i] = nuovaSessione;
                break;
            }
    }
    descriptors->used++;
    return nuovaSessione;
}

//---------------------------------------------------------------------------------------
void initializeSessionsDescritors(int maxFiles, int maxSize){
    store_initialize(maxFiles, maxSize);
    descriptors = malloc(sizeof (Array));
    descriptors->array = malloc(INITIALSIZE * sizeof(sessionDescriptor_t *));
    memset(descriptors->array, 0, INITIALSIZE * sizeof(sessionDescriptor_t *));
    descriptors->used = 0;
    descriptors->size = INITIALSIZE;
    pthread_mutex_init(&_filesessions_mutex, NULL);
}

//---------------------------------------------------------------------------------------
void deinitializeSessionsDescritors(){
    store_deinitialize();

    for (int i=0; i<descriptors->size; i++){
        freeDescriptor(i);
    }
    free(descriptors->array);
    free(descriptors);
    descriptors = NULL;
    pthread_mutex_unlock(&_filesessions_mutex);
    pthread_mutex_destroy(&_filesessions_mutex);
}

//---------------------------------------------------------------------------------------
int openFile(int connfd, char *pathname, int flags, char * dirname) {
    pthread_mutex_lock(&_filesessions_mutex);

    sessionDescriptor_t *sessionOfUser = findSession(connfd, pathname);

    if (sessionOfUser) { // file già aperto dal client
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_ALREADY_OPEN;
    }

    if (store_isLocked(pathname))   // file già locked
    {
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_ALREADY_LOCKED;
    }

    if (flags & O_CREATE){     // crezione del file
        int res = store_createFile(pathname,((flags & O_LOCK) != 0), dirname);
        if (res) {
            pthread_mutex_unlock(&_filesessions_mutex);
            return FILE_CREATE_NOT_POSSIBLE;
        }
        createSession(connfd, pathname,flags);
        pthread_mutex_unlock(&_filesessions_mutex);
        return 0;
    }

    if (!store_isPresent(pathname)){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NOT_FOUND;
    }

    createSession(connfd, pathname,flags);
    if (flags & O_LOCK)
    {
        store_lockFile(pathname);
    }
    pthread_mutex_unlock(&_filesessions_mutex);
    return 0;
}

//---------------------------------------------------------------------------------------
int closeFile(int connfd, char *pathname) {
    pthread_mutex_lock(&_filesessions_mutex);

    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (sessionOfUser->flags & O_LOCK)
        store_unlockFile(pathname);
    freeDescriptor(findSessionIndex(connfd,pathname));
    pthread_mutex_unlock(&_filesessions_mutex);
    return 0;
}

//---------------------------------------------------------------------------------------
int lockFile(int connfd, char *pathname){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (store_isLocked(pathname)){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_ALREADY_LOCKED;
    }
    sessionOfUser->flags |= O_LOCK;
    store_lockFile(pathname);
    pthread_mutex_unlock(&_filesessions_mutex);
    return 0;
}

//---------------------------------------------------------------------------------------
int unlockFile(int connfd, char *pathname){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (!(sessionOfUser->flags & O_LOCK))  // lock di altro client
    {
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_LOCKED;
    }
    store_unlockFile(pathname);
    pthread_mutex_unlock(&_filesessions_mutex);
    return 0;
}

//---------------------------------------------------------------------------------------
int removeFile(int connfd, char *pathname){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (!(sessionOfUser->flags & O_LOCK))  // lock di altro client
    {
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_LOCKED;
    }
    store_removeFile(pathname);
    freeDescriptor(findSessionIndex(connfd,pathname));
    pthread_mutex_unlock(&_filesessions_mutex);
    return 0;
}

//---------------------------------------------------------------------------------------
int getMaxFiles(){
    return store_maxFiles();
}

//---------------------------------------------------------------------------------------
int readNextFile(int idx, void **data, int* dataLen, char **pathname){
    return store_readNextFile(idx, data, dataLen, pathname);
}

//---------------------------------------------------------------------------------------
int readFileData(int connfd,char *pathname, void** data, int*sizeOfData){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    int retVal = store_readFile(pathname,data,sizeOfData);
    pthread_mutex_unlock(&_filesessions_mutex);
    return retVal;
}

//---------------------------------------------------------------------------------------
int writeFile(int connfd, char *pathname,
              void *buffer, int sizeOfData, char * dirname){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (!(sessionOfUser->flags & O_LOCK))  // lock di altro client
    {
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_LOCKED;
    }

    int retVal = store_writeFile(pathname,dirname,buffer,sizeOfData,false);
    pthread_mutex_unlock(&_filesessions_mutex);
    return retVal;
}

//---------------------------------------------------------------------------------------
int appendaFileData(int connfd, char *pathname, void *buffer, int sizeOfData ,char *dirname){
    pthread_mutex_lock(&_filesessions_mutex);
    sessionDescriptor_t* sessionOfUser = findSession(connfd, pathname);
    if (sessionOfUser == NULL){
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_OPEN;
    }
    if (!(sessionOfUser->flags & O_LOCK))  // lock di altro client
    {
        pthread_mutex_unlock(&_filesessions_mutex);
        return FILE_NEVER_LOCKED;
    }
    int retVal = store_writeFile(pathname,dirname,buffer,sizeOfData,true);
    pthread_mutex_unlock(&_filesessions_mutex);
    return  retVal;
}


