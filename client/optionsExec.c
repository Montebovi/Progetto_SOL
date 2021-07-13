#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "errors.h"
#include "server_api.h"
#include "opzioniman.h"
#include "optionsExec.h"
#include "../common/svc_commands.h"
#include "fileutilities.h"


//----------------------------------------------------------------------------------------
static char * getRealPath(char * pathname){
    char * absPath = realpath(pathname,NULL);
    if (absPath != NULL)
        errno =0;
    return absPath;
}

//----------------------------------------------------------------------------------------
static char ** tokenize(char *str, int *n){
    char ** tokens = malloc(100*sizeof (char*));
    char *tmpstr;
    char *token = strtok_r(str, ",", &tmpstr);
    *n=0;
    while (token) {
        if (*n >= 100)
            return NULL;
        tokens[(*n)++] = strdup(token);
        token = strtok_r(NULL, ",", &tmpstr);
    }
    return tokens;
}

//----------------------------------------------------------------------------------------
static int writeFileCore(char *pathname, struct optionsType opzioni) {
    int retVal;
    void *buffer;
    size_t dataLen;
    retVal = loadFile(pathname,&buffer, &dataLen);
    if (retVal == -1){
        return retVal;
    }

    char *dirname = NULL;
    if (opzioni.directoryMemSecondariaWhenCapacityMisses)
        dirname = getRealPath(opzioni.directoryMemSecondariaWhenCapacityMisses);

    retVal = writeFile(pathname, buffer,dataLen, dirname);
    free(dirname);
    free(buffer);
    if (retVal == -1)
        return retVal;

    return dataLen;
}

//----------------------------------------------------------------------------------------
int execute_open(char *arg,int mode,struct optionsType opzioni){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);

    char *dirname = NULL;
    if (opzioni.directoryMemSecondariaWhenCapacityMisses)
        dirname = getRealPath(opzioni.directoryMemSecondariaWhenCapacityMisses);

    for (int i = 0; i < nFiles; i++) {
        char *pathname = getRealPath(files[i]);
        retVal = openFile(pathname,mode,dirname);
        free(pathname);
        pathname = NULL;
        if (retVal == -1){
            free(dirname);
            errno = ERR_OPEN_FAILURE;
            return -1;
        }
    }
    free(dirname);
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_close(char *arg){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char *pathname = getRealPath(files[i]);
        retVal = closeFile(pathname);
        free(pathname);
        pathname = NULL;
        if (retVal == -1){
            errno = ERR_FILE_CLOSE_ERROR;
            return -1;
        }
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_remove(char *arg){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char *pathname = getRealPath(files[i]);
        retVal = removeFile(pathname);
        free(pathname);
        pathname = NULL;
        if (retVal == -1){
            errno = ERR_FILE_NOT_REMOVED;
            return -1;
        }
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_lock(char *arg){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char *pathname = getRealPath(files[i]);
        retVal = lockFile(pathname);
        free(pathname);
        pathname = NULL;
        if (retVal == -1){
            errno = ERR_LOCK_FILE_FAILURE;
            return -1;
        }
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_unlock(char *arg){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char *pathname = getRealPath(files[i]);
        retVal = unlockFile(pathname);
        free(pathname);
        pathname = NULL;
        if (retVal == -1){
            errno = ERR_UNLOCK_FILE_FAILURE;
            return -1;
        }
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_readN(char *arg,struct optionsType opzioni){
    if (opzioni.directoryScritturaFile == 0)
    {
        errno = ERR_DIR_NOT_SPECIFIED;
        return -1;
    }
    int numFiles = 0;
    if (arg != NULL)
        numFiles = atoi(arg);

    char *dirname = getRealPath(opzioni.directoryScritturaFile);
    int retVal = readNFiles(numFiles,dirname);
    while (retVal != -1)
    {
        char *pathname;
        void *data;
        int dataLen;
        retVal = readFileFromServer(&pathname,&data, &dataLen);
        if (dataLen == 0)
            break;
        saveFile(dirname,pathname,data,dataLen);
        free(data);
        free(pathname);
    }
    free(dirname);
    if (retVal == -1){
        errno = ERR_READ_ERROR;
        return -1;
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_r(char *arg, struct optionsType opzioni, void ** data, int* numReadBytes){
    int retVal = 0;
    int nFiles;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char* pathname = getRealPath(files[i]);
        *data = NULL;
        size_t sizeOfData = 0;
        retVal = readFile(pathname,data,&sizeOfData);
        *numReadBytes = sizeOfData;
        if (retVal == -1){
            free(pathname);
            pathname = NULL;
            errno = ERR_READ_ERROR;
            return -1;
        }

        if (sizeOfData > 0)
        {
            char *dirname = getRealPath(opzioni.directoryScritturaFile);
            saveFile(dirname,pathname,*data,(int)sizeOfData);
            free(dirname);
        }

        free(pathname);
        pathname = NULL;
    }
    return retVal;
}

//----------------------------------------------------------------------------------------
int execute_w(char *arg, struct optionsType opzioni, int *totWrittenBytes){
    int nParams;
    char **params = tokenize(arg,&nParams);
    *totWrittenBytes=0;
    if (nParams == 0 || nParams > 2)
    {
        errno = ERR_INVALID_NUMBER_OF_ARGUMENTS;
        return -1;
    }
    char* fullDirName = getRealPath(params[0]);

    if (!directoryExists(fullDirName))
    {
        errno = ERR_DIRECTORY_NOT_FOUND;
        free(fullDirName);
        return -1;
    }

    int numFiles = 0;
    if (nParams > 1)
        numFiles = atoi(params[1]);

    if (numFiles == 0)
        numFiles = getNumOfFilesOfDirectoryRec(fullDirName);


    if (numFiles == 0) { // non ci sono file sin directory
        free(fullDirName);
        return 0;
    }

    char *dirname = NULL;
    if (opzioni.directoryMemSecondariaWhenCapacityMisses)
        dirname = getRealPath(opzioni.directoryMemSecondariaWhenCapacityMisses);

    char *nomifile[numFiles];
    int idx=0;
    getFilesOfDirectoryRec(fullDirName,nomifile,&idx,numFiles);
    free(fullDirName);
    for(int i=0; i<idx; i++){
        int retVal = openFile(nomifile[i],O_CREATE|O_LOCK,dirname);
        if (retVal == -1) {
            //todo: errno
            free(dirname);
            return -1;
        }

        int writtenBytes = writeFileCore(nomifile[i],opzioni);
        if (writtenBytes == -1) {
            free(dirname);
            free(nomifile[i]);
            //todo: errno
            return -1;
        }

        retVal = closeFile(nomifile[i]);
        if (retVal == -1) {
            free(dirname);
            free(nomifile[i]);
            //todo: errno
            return -1;
        }
        free(nomifile[i]);

        *totWrittenBytes += writtenBytes;
    }
    free(dirname);
    return 0;
}

//----------------------------------------------------------------------------------------
int execute_W(char *arg, struct optionsType opzioni, int *totWrittenBytes){
   *totWrittenBytes=0;
    int nFiles;
    int retVal;
    char **files = tokenize(arg,&nFiles);
    for (int i = 0; i < nFiles; i++) {
        char* pathname = getRealPath(files[i]);
        retVal = writeFileCore(pathname,opzioni);
        free(pathname);
        pathname=NULL;
        if (retVal == -1)
            return retVal;
        *totWrittenBytes+=retVal;
    }
    return 0;
}