#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include "conn.h"
#include "util.h"
#include "errors.h"
#include "../common/svc_commands.h"

static int sockfd = -1;

//----------------------------------------------------------------------------------
static void resetCommand(commandToSvr_t *cmd){
    memset(cmd,0,sizeof (commandToSvr_t));
}

//----------------------------------------------------------------------------------
static int receiveCmdRes(){
    int cmdRes;
    int n = readn(sockfd, &cmdRes, sizeof(int));
    if (n == -1){
        errno = ERR_RECEIVE_DATA;
        return -1;
    }
    return cmdRes;
}

//----------------------------------------------------------------------------------
static int receiveData(void** buf, size_t* size){
    int len;
    int n = readn(sockfd, &len, sizeof(int));
    *size = len;

    *buf = malloc(len);
    if (!*buf) {
        errno = ERR_OUT_OF_MEMORY;
        return -1;
    }

    n = readn(sockfd, *buf, len);
    if (n == -1){
        errno = ERR_RECEIVE_DATA;
        return -1;
    }
    return 0;
}

//----------------------------------------------------------------------------------
static int sendCommand(commandToSvr_t cmd) {
    char str[500];
    sprintf(str, "%d|%d|%d|%d|%s|%s", cmd.cmd, cmd.flags, cmd.sizeData, cmd.numFiles, cmd.pathname, cmd.dirname);
    int n = strlen(str);
    int retval = writen(sockfd,&n,sizeof (int));
    if (retval == -1){
        errno = ERR_SEND_DATA;
        return -1;
    }
    retval = writen(sockfd,str,n*sizeof (char));
    if (retval == -1){
        errno = ERR_SEND_DATA;
        return -1;
    }
    if (cmd.data != NULL) {
        retval = writen(sockfd,cmd.data,cmd.sizeData);
        if (retval == -1){
            errno = ERR_SEND_DATA;
            return -1;
        }
    }
    return 0;
}


//------------------------------------------------------------------------------------------------
// openConnection
int openConnection(const char *sockname, int msec, const struct timespec abstime) {
    struct sockaddr_un serv_addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1){
        errno = ERR_CONNECTION_NOT_OPEN;
        return -1;
    }
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, sockname, strlen(sockname) + 1);

    while (1) {
        printf("Tentativo di connessione...\n");
        time_t rawtime;
        time(&rawtime);
        if (rawtime > abstime.tv_sec){
            errno = ERR_CONNECTION_NOT_OPEN;
            return -1;
        }

        int retVal = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if (retVal == -1) {
            usleep(msec * 1000); // attesa 1 sec.
            continue;
        }
        return 0;
    }
}

//------------------------------------------------------------------------------------------------
// closeConnection
int closeConnection(const char* sockname) {
    int fail = shutdown(sockfd, SHUT_RDWR);
    if (fail)
    {
        errno = ERR_CONNECTION_NOT_CLOSED;
        return -1;
    }

    fail = close(sockfd);
    if (fail)
    {
        errno = ERR_CONNECTION_NOT_CLOSED;
        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------------------------
// readFile
int readFileFromServer(char **pathname, void **data, int *datLen){
    int lenPathname;
    int n = readn(sockfd, &lenPathname, sizeof(int));
    if (lenPathname == 0)
    {
        *pathname=NULL;
        *data = NULL;
        *datLen = 0;
        return 0;
    }
    *pathname = malloc(lenPathname);
    readn(sockfd,*pathname,lenPathname*sizeof(char));

    n = readn(sockfd, datLen, sizeof(int));
    *data = malloc(*datLen);
    n = readn(sockfd, *data, *datLen);
    return n;
}

//------------------------------------------------------------------------------------------------
// readFile
int readFile(const char* pathname, void** buf, size_t* size){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_READ_FILE;
    cmd.flags = -1;
    cmd.pathname = strdup(pathname);
    int res = sendCommand(cmd);
    if (res == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes == 0) {
        int retVal = receiveData(buf, size);
        return retVal;
    }
    return -1;
}

//------------------------------------------------------------------------------------------------
// removeFile
int removeFile(const char* pathname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_REMOVE_FILE;
    cmd.flags = -1;
    cmd.pathname = strdup(pathname);
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// lockFile
int lockFile(const char* pathname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_LOCK_FILE;
    cmd.flags = -1;
    cmd.pathname = strdup(pathname);
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// unlockFile
int unlockFile(const char* pathname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_UNLOCK_FILE;
    cmd.flags = -1;
    cmd.pathname = strdup(pathname);
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// closeFile
int closeFile(const char* pathname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_CLOSE_FILE;
    cmd.flags = -1;
    cmd.pathname =  strdup(pathname);
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// openFile
int openFile(const char* pathname, int flags, const char* dirname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_OPEN_FILE;
    cmd.flags = flags;
    cmd.pathname = strdup(pathname);
    if (dirname)
        cmd.dirname = strdup(dirname);
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// writeFile
int writeFile(const char* pathname, void *buf, size_t size, const char* dirname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_WRITE_FILE;

    cmd.pathname = strdup(pathname);
    if (dirname)
      cmd.dirname = strdup(dirname);
    cmd.sizeData = size;
    cmd.data = buf;
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// readNFiles
int readNFiles(int n, const char* dirname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_READ_N_FILE;
    cmd.dirname = strdup(dirname);
    cmd.numFiles = n;
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}

//------------------------------------------------------------------------------------------------
// appendToFile
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    commandToSvr_t cmd;
    resetCommand(&cmd);
    cmd.cmd = CMD_APPEND_TO_FILE;
    cmd.pathname = strdup(pathname);
    cmd.dirname = strdup(dirname);
    cmd.sizeData = size;
    cmd.data = buf;
    int retVal = sendCommand(cmd);
    if (retVal == -1)
        return -1;
    int cmdRes = receiveCmdRes();
    if (cmdRes != 0)
        return -1;
    return 0;
}
