#include <malloc.h>
#include <stdbool.h>
#include <bits/types/time_t.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "../common/logFile.h"
#include "filecache.h"

static int _maxFiles = -1;
static int _maxSizeInMB = -1;
static int _numEspulsi = 0;

#define MB (1024*1024.0)

typedef struct {
    char * pathname;
    int sizeOfFile;
    bool isLocked;
    void *contenuto;
    time_t lastAccessTime;
} fileItem_t ;

typedef struct {
    int totalSize ;
    int numFiles;
    int numEspulsi;
} fileStorageStatus_t;

static fileItem_t ** _entries;
static  pthread_mutex_t _store_mutex;


static void freeEntry(int idx) {
    if (_entries[idx] != 0){
        free(_entries[idx]->pathname);
        free(_entries[idx]->contenuto);
        _entries[idx]->pathname = NULL;
        _entries[idx]->contenuto = NULL;
        free(_entries[idx]);
        _entries[idx] = NULL;
    }
}

//-------------------------------------------------------------------------------
static void freeEntries(){
    for (int i=0; i<_maxFiles; i++)
        freeEntry(i);
}

//-------------------------------------------------------------------------------
static int findFreeItem(){
    for (int i=0; i<_maxFiles; i++){
        if (_entries[i] == 0)
            return i;
    }
    return -1;
}

//-------------------------------------------------------------------------------
static void currentStatus(fileStorageStatus_t *status){
    status->totalSize = 0;
    status->numFiles = 0;
    status->numEspulsi = _numEspulsi;
    for (int i=0; i<_maxFiles; i++){
        if (_entries[i] != 0){
            status->totalSize += _entries[i]->sizeOfFile;
            status->numFiles += 1;
        }
    }
}

//-------------------------------------------------------------------------------
static time_t getNowTime(){
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}

//-------------------------------------------------------------------------------
static int findItem(char* pathname) {
    for (int i=0; i<_maxFiles; i++){
        if (_entries[i] != 0 && (strcmp(_entries[i]->pathname, pathname) == 0))
            return i;
    }
    return -1;
}

//-------------------------------------------------------------------------------
static int getOldFile(int idxDenied) {
    int idx = -1;
    time_t  lastAccessTimeFound;
    for (int i = 0; i < _maxFiles; i++) {
        if (_entries[i] == 0)
            continue;
        if (i == idxDenied)
            continue;
        if (_entries[i]->isLocked)
            continue;
        if (idx == -1 || _entries[i]->lastAccessTime < lastAccessTimeFound){
            idx = i;
            lastAccessTimeFound = _entries[i]->lastAccessTime;
        }
    }
    return idx;
}

//-------------------------------------------------------------------------------
static int saveFileTo(int idx, char *dirname){
    char * filename = getFileNameFromPath(_entries[idx]->pathname);
    char* fullFilename = combinePathAndFile(dirname,filename);
    FILE *f_dst;
    f_dst  = fopen (fullFilename, "wb");
    if (f_dst == NULL){
        free(fullFilename);
        fullFilename = NULL;
        return -1;
    }

    if (fwrite(_entries[idx]->contenuto, 1, _entries[idx]->sizeOfFile,f_dst) != _entries[idx]->sizeOfFile)
    {
        free(fullFilename);
        return -1;
    }
    fclose(f_dst);
    f_dst = NULL;

    free(fullFilename);
    return 0;
}

//-------------------------------------------------------------------------------
static int dischargOneFile(char *dirname){
    int idx = getOldFile(-1);
    if (dirname != NULL){
        int retVal = saveFileTo(idx,dirname);
        if (retVal == -1)
            return -1;
    }
    _numEspulsi++;
    freeEntry(idx);
    return idx;
}

//-------------------------------------------------------------------------------
static int dischargeFiles(int idxDenied, int requiredSize, char *dirname){
    fileStorageStatus_t status;
    currentStatus(&status);
    int freeBytes = _maxSizeInMB*MB- status.totalSize;
    while (freeBytes < requiredSize){
        int idx = getOldFile(idxDenied);
        freeBytes += _entries[idx]->sizeOfFile;
        if (dirname != NULL){
            int retVal = saveFileTo(idx,dirname);
            if (retVal == -1)
                return -1;
        }
        _numEspulsi++;
        freeEntry(idx);
    }
    return 0;
}

//-------------------------------------------------------------------------------
static bool store_isPresentInternal(char* pathname){
    int idx = findItem(pathname);
    return (idx >= 0);
}

//**********************************************************************************************************************
//**********************************************************************************************************************
//**********************************************************************************************************************

//-------------------------------------------------------------------------------
int store_initialize(int maxFiles, int maxSize) {
    _numEspulsi = 0;
    _maxFiles = maxFiles;
    _maxSizeInMB = maxSize;
    if (_entries != NULL){
        store_deinitialize();
    }
    _entries = (fileItem_t ** )malloc(maxFiles*sizeof(fileItem_t *));
    memset(_entries,0,maxFiles*sizeof(fileItem_t *));
    pthread_mutex_init(&_store_mutex, NULL);
    return 0;
}

//-------------------------------------------------------------------------------
void store_deinitialize(){
    pthread_mutex_lock(&_store_mutex);

    freeEntries();
    free(_entries);
    _entries = NULL;

    pthread_mutex_unlock(&_store_mutex);
    pthread_mutex_destroy(&_store_mutex);
}

//-------------------------------------------------------------------------------
int store_maxFiles(){
    return _maxFiles;
}


//-------------------------------------------------------------------------------
void printStatus(){
    fileStorageStatus_t status;

    pthread_mutex_lock(&_store_mutex);
    currentStatus(&status);
    pthread_mutex_unlock(&_store_mutex);

    printf("**** STATO FILE STORAGE ****\n");
    if (status.totalSize < 1024)
    {
        logMsgf("MEMORIA CACHE: Memoria occupata: %d bytes su %d MB",status.totalSize,_maxSizeInMB);
        printf("Memoria occupata: %d bytes su %d MB\n",status.totalSize,_maxSizeInMB);
    }
    else{
        logMsgf("MEMORIA CACHE: Memoria occupata: %.2f MB su %d MB",status.totalSize/MB,_maxSizeInMB);
        printf("Memoria occupata: %.2f MB su %d MB\n",status.totalSize/MB,_maxSizeInMB);
    }
    logMsgf("NUM FILES in CACHE: %d su %d", status.numFiles,_maxFiles);
    logMsgf("NUM FILES totali esplusi: %d", status.numEspulsi);
    printf("Numero files: %d su %d\n", status.numFiles,_maxFiles);
    printf("****************************\n");
}

//-------------------------------------------------------------------------------
bool store_isPresent(char* pathname){
    pthread_mutex_lock(&_store_mutex);
    bool res = store_isPresentInternal(pathname);
    pthread_mutex_unlock(&_store_mutex);
    return res;
}

//-------------------------------------------------------------------------------
int store_createFile(char *pathname, bool isLocked,char *dirname){
    pthread_mutex_lock(&_store_mutex);
    if (store_isPresentInternal(pathname))
    {
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    int idx = findFreeItem();
    if (idx == -1){    // il file store è pieno
        idx = dischargOneFile(dirname);
        if (idx == -1)
        {
            pthread_mutex_unlock(&_store_mutex);
            return -1;
        }
    }
    fileItem_t * nuovoFile = malloc(sizeof (fileItem_t));
    nuovoFile->pathname = strdup(pathname);
    nuovoFile->isLocked = isLocked;
    nuovoFile->sizeOfFile = 0;
    nuovoFile->contenuto = NULL;
    nuovoFile->lastAccessTime =  getNowTime();

    _entries[idx] = nuovoFile;
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
int store_lockFile(char* pathname){
    pthread_mutex_lock(&_store_mutex);

    int idx = findItem(pathname);
    if (idx == -1){
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    if (_entries[idx]->isLocked)
    {
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }

    _entries[idx]->isLocked = 1;
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
int store_removeFile(char* pathname){
    pthread_mutex_lock(&_store_mutex);
    int idx = findItem(pathname);
    if (idx == -1)
    {
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    freeEntry(idx);
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
int store_unlockFile(char* pathname){
    pthread_mutex_lock(&_store_mutex);
    int idx = findItem(pathname);
    if (idx == -1){
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    _entries[idx]->isLocked = 0;
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
bool store_isLocked(char* pathname){
    pthread_mutex_lock(&_store_mutex);
    int idx = findItem(pathname);
    if (idx == -1){
        pthread_mutex_unlock(&_store_mutex);
        return false;
    }
    pthread_mutex_unlock(&_store_mutex);
    return _entries[idx]->isLocked;
}

//-------------------------------------------------------------------------------
int store_readFile(char* pathname , void** data, int*sizeOfData){
    pthread_mutex_lock(&_store_mutex);

    int idx = findItem(pathname);
    if (idx == -1)
    {
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    *sizeOfData =_entries[idx]->sizeOfFile;
    if (*sizeOfData == 0){
        *data = NULL;
    }
    else
    {
        *data = malloc(*sizeOfData);
        memcpy(*data,_entries[idx]->contenuto,*sizeOfData);
    }
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
int store_writeFile(char* pathname, char *dirname, void *buffer, size_t len, bool appendMode){
    pthread_mutex_lock(&_store_mutex);
    int idx = findItem(pathname);
    if (idx == -1){
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }
    if (!_entries[idx]->isLocked){
        pthread_mutex_unlock(&_store_mutex);
        return -1;
    }

    fileStorageStatus_t status;
    currentStatus(&status);
    if (_maxSizeInMB*MB < len){
        pthread_mutex_unlock(&_store_mutex);
        return -1; // il file non entra in ogni caso
    }
    else if (_maxSizeInMB*MB < status.totalSize+len)
    {
        int memoriaLiberata = dischargeFiles(idx, len, dirname);
        if (memoriaLiberata == -1){
            pthread_mutex_unlock(&_store_mutex);
            return -1;
        }
    }
    if (!appendMode && _entries[idx]->contenuto != 0){
        free(_entries[idx]->contenuto);
        _entries[idx]->contenuto = NULL;
        _entries[idx]->sizeOfFile = 0;
    }

    if  (appendMode && _entries[idx]->sizeOfFile != 0){
        _entries[idx]->contenuto = realloc(_entries[idx]->contenuto, _entries[idx]->sizeOfFile+len);
        memcpy(((char*)_entries[idx]->contenuto)+_entries[idx]->sizeOfFile,buffer,len);
        _entries[idx]->sizeOfFile += len;
    }
    else {
        _entries[idx]->contenuto = malloc(len);
        _entries[idx]->sizeOfFile = len;
        memcpy(_entries[idx]->contenuto, buffer,len);
    }
    _entries[idx]->lastAccessTime =  getNowTime();
    pthread_mutex_unlock(&_store_mutex);
    return 0;
}

//-------------------------------------------------------------------------------
int store_readNextFile(int idx, void **data, int* dataLen, char **pathname){
    pthread_mutex_lock(&_store_mutex);
    while (idx < _maxFiles)
    {
        if (_entries[idx] != 0 && _entries[idx]->sizeOfFile > 0){
            *pathname = strdup(_entries[idx]->pathname);
            *dataLen =_entries[idx]->sizeOfFile;
            *data = malloc(*dataLen);
            memcpy(*data,_entries[idx]->contenuto,*dataLen);
            break;
        }
            idx++;
    }
    pthread_mutex_unlock(&_store_mutex);
    if (idx >= _maxFiles)
        return -1;
    return idx;
}

