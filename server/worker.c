#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "conn.h"
#include "util.h"
#include "queue.h"
#include "svc_request.h"
#include "filecache.h"
#include "filesessions.h"
#include "../common/svc_commands.h"
#include "../common/logFile.h"
#include "worker.h"
#include "cond_manager.h"

static volatile bool endServer = false;

void endWorkers(){
    endServer = true;
}

int executeRequest(int threadId, request_t *req, void **data, int* sizeOfData);

//-----------------------------------------------------------------------------------------------
int writeFileToClient(int connfd, char * pathname, void *data, int dataLen){
    int retval;
    if (pathname == NULL){
        int v = 0;
         retval = writen(connfd, &v, sizeof(int));
    }
    else {
        int size = strlen(pathname)+1;
        int retval = writen(connfd, &size, sizeof(int));
        if (retval == -1) return -1;
        retval = writen(connfd, pathname, size);
        if (retval == -1) return -1;
        retval = writen(connfd, &dataLen, sizeof(int));
        if (retval == -1) return -1;
        retval = writen(connfd, data, dataLen);
    }
    return retval;
}

//-----------------------------------------------------------------------------------------------
void writeCmdRes(int connfd, int cmdRes){
    int retval = writen(connfd, &cmdRes, sizeof(int));
    if (retval == -1) {
        //manage error
    }
}

//-----------------------------------------------------------------------------------------------
void *handle_client_request(/*workerParams_t*/ void  * workerParamsPrm) {
    workerParams_t *workerParams = (workerParams_t*)workerParamsPrm;
    printf("Hi worker %d\n",workerParams->threadId);
    request_t *req = malloc(sizeof(request_t));
    memset(req,0,sizeof(request_t));

    while (!endServer) {
        int ret = QUEUE_dequeue(workerParams->queue, req, sizeof(request_t));
        if (ret == QUEUE_RET_QueueIsEmpty) {
            usleep(100 * 1000);  //100 ms
        } else {
            void * data = NULL;
            int sizeOfData;
            int retVal = executeRequest(workerParams->threadId, req, &data, &sizeOfData);
            if (retVal != 0)
            {
                freeRequest(req);
                req = malloc(sizeof(request_t));
                memset(req,0,sizeof(request_t));
                continue; // salta la richiesta
            }
            if (data != NULL){
                int retval = writen(req->connDescr, &sizeOfData, sizeof(int));
                if (retval == -1) {
                    free(data);
                    freeRequest(req);
                    continue;
                }
                if (sizeOfData > 0){
                    int retval = writen(req->connDescr, data,sizeOfData);
                    if (retval == -1) {
                        free(data);
                        freeRequest(req);
                        continue;
                    }
                }
                free(data);
                data = NULL;
            }
            freeRequest(req);
            req = malloc(sizeof(request_t));
            memset(req,0,sizeof(request_t));
        }
    }
    freeRequest(req);
    req = NULL;
    logMsgf("worker (%d) - terminato",workerParams->threadId);
    printf("worker (%d) - terminato\n",workerParams->threadId);
    return 0;
}

//---------------------------------------------------------------------------------------
int readNFiles(int connfd,int numFiles){
    writeCmdRes(connfd,0);

    if (numFiles == 0)
        numFiles = getMaxFiles();

    int res = 0;
    int idxBase = 0;
    while (idxBase >= 0 && numFiles > 0){
        void *data;
        char *pathname;
        int dataLen;
        idxBase = readNextFile(idxBase,&data, &dataLen, &pathname);
        if (idxBase == -1){
            break;
        }
        else{
             res = writeFileToClient(connfd, pathname,data,dataLen);
            free(data);
            free(pathname);
            if (res == -1)
            {
                // to manage
            }
            idxBase++;
            numFiles--;
        }
    }
    res = writeFileToClient(connfd,NULL,NULL,0);
    return res;
}

//------------------------------------------------------------------------------------------------
int executeRequest(int threadId, request_t *req, void **data, int* sizeOfData) {
    int retVal = -1;
    *data=0;
    *sizeOfData = 0;
    switch (req->cmdDescr->cmd) {
        case CMD_OPEN_FILE:{
            logMsgf("[worker(%d)] openfile %s flags:%d",threadId,req->cmdDescr->pathname,req->cmdDescr->flags);
            printf("[worker(%d)] openfile\n",threadId);
            while (true) {
                retVal = openFile(req->connDescr, req->cmdDescr->pathname, req->cmdDescr->flags, req->cmdDescr->dirname);
                if (retVal == FILE_ALREADY_LOCKED) {
                    printf("open wait....\n");
                    cond_wait(req->cmdDescr->pathname);
                }
                else
                    break;
            }
            writeCmdRes(req->connDescr, retVal);
            break;
        }
        case CMD_CLOSE_FILE:{
            logMsgf("[worker(%d)] closefile %s",threadId,req->cmdDescr->pathname);
            printf("[worker(%d)] closefile\n",threadId);
            retVal = closeFile(req->connDescr,req->cmdDescr->pathname);
            writeCmdRes(req->connDescr,retVal);
            cond_signal(req->cmdDescr->pathname);
            break;
        }
        case CMD_LOCK_FILE:{
            logMsgf("[worker(%d)] lockfile %s",threadId,req->cmdDescr->pathname);
            printf("[worker(%d)] lockfile\n",threadId);
            while (true){
                retVal = lockFile(req->connDescr,req->cmdDescr->pathname);
                if (retVal == FILE_ALREADY_LOCKED){
                    logMsgf("[worker(%d)] lockfile WAIT %s",threadId,req->cmdDescr->pathname);
                    printf("lock wait....\n");
                    cond_wait(req->cmdDescr->pathname);
                }
                else
                  break;
            }
            writeCmdRes(req->connDescr,retVal);
            break;
        }
        case CMD_UNLOCK_FILE:{
            logMsgf("[worker(%d)] unlockfile %s",threadId,req->cmdDescr->pathname);
            printf("[worker(%d)] unlockfile\n",threadId);
             retVal = unlockFile(req->connDescr,req->cmdDescr->pathname);
            writeCmdRes(req->connDescr,retVal);
            cond_signal(req->cmdDescr->pathname);
            break;
        }
        case CMD_REMOVE_FILE:{
            logMsgf("[worker(%d)] remove file %s",threadId,req->cmdDescr->pathname);
            printf("[worker(%d)] remove file\n",threadId);
             retVal = removeFile(req->connDescr,req->cmdDescr->pathname);
            writeCmdRes(req->connDescr,retVal);
             break;
        }
        case CMD_READ_N_FILE:{
            logMsgf("[worker(%d)] read N files  (N=%d)",threadId,req->cmdDescr->numFiles);
            printf("[worker(%d)] read N files\n",threadId);
            retVal = readNFiles(req->connDescr,req->cmdDescr->numFiles);
            break;
        }
        case CMD_READ_FILE:{
            logMsgf("[worker(%d)] read file data %s",threadId,req->cmdDescr->pathname);
            printf("[worker(%d)] read file data\n",threadId);
            retVal = readFileData(req->connDescr,req->cmdDescr->pathname, data, sizeOfData);
            writeCmdRes(req->connDescr,retVal);
            break;
        }
        case CMD_WRITE_FILE:{
            logMsgf("[worker(%d)] write file data %s (size of data: %d)",threadId,req->cmdDescr->pathname,req->cmdDescr->sizeData);
            printf("[worker(%d)] write file\n",threadId);
            retVal = writeFile(req->connDescr,req->cmdDescr->pathname,req->cmdDescr->data,req->cmdDescr->sizeData, req->cmdDescr->dirname);
            writeCmdRes(req->connDescr,retVal);
            break;
        }
        case CMD_APPEND_TO_FILE:{
            int  sizeData = req->cmdDescr->sizeData;
            logMsgf("[worker(%d)] append file data %s (size of data: %d)",threadId,req->cmdDescr->pathname,sizeData);
            printf("[worker(%d)] append to file data size = %d\n",threadId,sizeData);
            retVal = appendaFileData(req->connDescr,req->cmdDescr->pathname,req->cmdDescr->data,req->cmdDescr->sizeData,req->cmdDescr->dirname);
            writeCmdRes(req->connDescr,retVal);
            break;
        }
    }
    printStatus();
    return retVal;
}
