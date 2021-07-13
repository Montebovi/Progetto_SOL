#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "conn.h"
#include "util.h"
#include "worker.h"
#include "configuration.h"
#include "queue.h"
#include "svc_request.h"
#include "filesessions.h"
#include "../common/svc_commands.h"
#include "../common/logFile.h"
#include "cond_manager.h"

static struct serverConfiguration configuration;

void cmdToStr(int cmd, char *str);

void create_worker(pthread_t *workerArr, int *threadId_arr, workerParams_t *parameWorkers,
                   int numWorkers, queue_t *pQueue);

void join_workers(pthread_t *workerArr, int numWorkers);


//------------------------------------------------------------------------------------------
// cleanup
void cleanup() {
    unlink(configuration.sockname);
}

//------------------------------------------------------------------------------------------
int receiveCommand(int connfd, commandToSvr_t *cmd) {
    msg_t msg;
    int n;

    n = readn(connfd, &msg.len, sizeof(int));
    if (n == -1) {
        //todo: gest errore
    }

    if (n == 0) {
        cmd->cmd = CMD_EOF;
        return 0;
    }

    msg.str = calloc((msg.len)+1, sizeof(char));
    if (!msg.str) {
        perror("calloc");
        fprintf(stderr, "Memoria esaurita....\n");
        free(msg.str);
        return 0;
    }
    n = readn(connfd, msg.str, msg.len * sizeof(char));

    char *str = msg.str;
    char *saveptr;

    char *token = strtok_r(str, "|",&saveptr);
    if (token != NULL)
        cmd->cmd = atoi(token);

    token = strtok_r(NULL, "|",&saveptr);
    if (token != NULL) {
        cmd->flags = atoi(token);
    }

    token = strtok_r(NULL, "|",&saveptr);
    if (token != NULL) {
        cmd->sizeData = atoi(token);
    }

    token = strtok_r(NULL, "|",&saveptr);
    if (token != NULL) {
        cmd->numFiles = atoi(token);
    }

    token = strtok_r(NULL, "|",&saveptr);
    if (token != NULL) {
        cmd->pathname = strdup(token); // malloc(strlen(token) + 1);
        //strcpy(cmd->pathname, token);
    }

    token = strtok_r(NULL, "|",&saveptr);
    if (token != NULL && strcmp(token,"(null)") != 0) {
        cmd->dirname = strdup(token);// malloc(strlen(token) + 1);
        //strcpy(cmd->dirname, token);
    }
    else
        cmd->dirname = NULL;

    if (cmd->sizeData > 0) {
        cmd->data = malloc(cmd->sizeData);
        n = readn(connfd, cmd->data, cmd->sizeData);
        if (n == -1) {
            free(msg.str);
            msg.str = NULL;
            //todo: gest errore
        }
    }

    free(msg.str);
    msg.str = NULL;

    return 0;
}

static volatile int _sigint = 0;
static volatile int _sigquit = 0;
static volatile int _sighup = 0;

//------------------------------------------------------------------------------------------
static void sigintHandler(int sig) {
    _sigint = 1;
}

//------------------------------------------------------------------------------------------
static void sigquitHandler(int sig) {
    _sigquit = 1;
}

//------------------------------------------------------------------------------------------
static void sighupHandler(int sig) {
    _sighup = 1;
}


#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)


//------------------------------------------------------------------------------------------
int connessioniChiuse(int fdNum){
    for (int fd = 0; fd<=fdNum;fd++) {
        char s[200];
        recv(fd,s,1, MSG_PEEK | MSG_DONTWAIT);
        if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
            return 0;  // si assume che la connessione esiste
    }
    return 1;
}

//------------------------------------------------------------------------------------------
// RUN DEL SERVER
int run_server(int argc, char *argv[]) {

    if (signal(SIGINT, sigintHandler) == SIG_ERR)
        errExit("signal SIGINT");

    if (signal(SIGQUIT, sigquitHandler) == SIG_ERR)
        errExit("signal SIGQUIT");

    if (signal(SIGHUP, sighupHandler) == SIG_ERR)
        errExit("signal SIGHUP");

    logMsg("Lettura file di configurazione");

    readConfig("configuration.txt", &configuration);
    showConfig(configuration);

    cleanup();

    initializeSessionsDescritors(configuration.maxFiles, configuration.maxSize);
    queue_t *queue;
    queue_ret_t ret;
    ret = QUEUE_initialize(&queue);
    if (ret != QUEUE_RET_Success)
        exit(1);

    cond_init();

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    int fd_c, fd_num = 0, fd;
    fd_set set, rdset;

    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, configuration.sockname, strlen(configuration.sockname) + 1);

    printf("[server] listening...\n");

    int notused;
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)), "bind", "");
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");
    if (listenfd > fd_num) fd_num = listenfd;

    FD_ZERO(&set);
    FD_SET(listenfd, &set);

    printf("[server] before while...\n");

    pthread_t workerArr[configuration.threadsWorkers];
    int threadId_arr[configuration.threadsWorkers];
    workerParams_t parametriWorkers[configuration.threadsWorkers];

    create_worker(workerArr, threadId_arr, parametriWorkers, configuration.threadsWorkers, queue);
    int msgSigHUP = 0;
    while(1) {
        if (_sigint != 0 || _sigquit != 0)
            break;
        if (_sighup != 0)
        {
            if (msgSigHUP == 0){
                logMsg("SIGHUP rilevato - in attesa di chiusura connessioni");
                msgSigHUP = 1;
            }
            int tutteChiuse = connessioniChiuse(fd_num);
            if (tutteChiuse){
                logMsg("Rilevato tutte le connessioni chiuse");
                break;
            }
        }
        rdset=set;
        logMsg("In attesa di comandi");
        if (select(fd_num+1,&rdset,NULL,NULL,NULL)==-1)    {
             // todo: gest errore
        }
        else{
            for (fd = 0; fd<=fd_num;fd++) {
                if (FD_ISSET(fd,&rdset)) {
                    if (fd== listenfd) {/* sock connect pronto */
                        if (_sighup == 0) {
                            logMsg("Accept connessione da client");
                            SYSCALL_EXIT("accept", fd_c, accept(listenfd, (struct sockaddr *) NULL, NULL), "accept",
                                         "");
                            FD_SET(fd_c, &set);
                            if (fd_c > fd_num) fd_num = fd_c;
                        }
                    }
                    else {/* sock I/0 pronto */
                        commandToSvr_t *cmd = malloc(sizeof(commandToSvr_t));
                        memset(cmd,0, sizeof(commandToSvr_t));
                        receiveCommand(fd,cmd);
                        if (cmd->cmd==CMD_EOF) {/* EOF client finito */
                            logMsg("EOF da client");
                            freeCommand(cmd);
                            cmd = NULL;
                            FD_CLR(fd,&set);
                            close(fd);
                        }
                        else{
                            char cmdStr[200];
                            cmdToStr(cmd->cmd,cmdStr);
                            logMsgf("Lettura comando: %s (%d) - %s",cmdStr,cmd->cmd,cmd->pathname);
                            request_t *req = malloc(sizeof(request_t));
                            memset(req,0, sizeof(request_t));

                            req->cmdDescr = cmd;
                            req->connDescr = fd;
                            QUEUE_enqueue(queue,req);
                            int qsize = QUEUE_size(queue);
                            logMsgf("Dimensione corrente coda dei comandi; %d",qsize);
                        }
                    }
                }
            }
        }
    }

    endWorkers();
    logMsg("Attesa conclusione workers");
    join_workers(workerArr, configuration.threadsWorkers);
    logMsg("Workers terminati");

    if (_sigint)
    {
        printf("Uscito SIGINT=%d\n", _sigint);
        logMsgf("Uscito SIGINT=%d\n", _sigint);
    }
    else if (_sigquit){
        printf("Uscito SIGQUIT=%d\n", _sigquit);
        logMsgf("Uscito SIGQUIT=%d\n", _sigquit);
    }
    else if (_sighup){
        printf("Uscito SIGHUP=%d\n", _sigquit);
        logMsgf("Uscito SIGHUP=%d\n", _sigquit);
    }

    QUEUE_free(queue);
    queue = NULL;
    deinitializeSessionsDescritors();

    cond_deinit();

    freeConfiguration(configuration);

    return 0;
}

//------------------------------------------------------------------------------------------
void join_workers(pthread_t *workerArr, int numWorkers) {
    void *status_ptr;
    for (int threadIndex = 0; threadIndex < numWorkers; threadIndex++) {
        pthread_join(workerArr[threadIndex], &status_ptr);
    }
}

//------------------------------------------------------------------------------------------
void create_worker(pthread_t *workerArr, int *threadId_arr, workerParams_t *parameWorkers, int numWorkers, queue_t *pQueue) {
    int threadIndex;
    int returnVal;
    //char message[100];

    logMsgf("Creazione %d workers",numWorkers);

    for (threadIndex = 0; threadIndex < numWorkers; threadIndex++) {
        threadId_arr[threadIndex] = threadIndex;
        parameWorkers[threadIndex].threadId = threadId_arr[threadIndex];
        parameWorkers[threadIndex].queue = pQueue;
        returnVal = pthread_create(&workerArr[threadIndex], NULL,
                                   handle_client_request, /*&threadId_arr[threadIndex]*//*pQueue*/
                                   &parameWorkers[threadIndex]);
        if (returnVal) {
            logMsgf("pthread_create() fails with error code %d", returnVal);
            exit(1);
        }
    }
}

