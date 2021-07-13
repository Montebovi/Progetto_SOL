#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "logFile.h"
#include "operationsReport.h"



//---------------------------------------------------------------------------------
static void opToStr(int op, char *operation) {
    switch (op) {
        case 'o': strcpy(operation,"OPEN");break;
        case 'b': strcpy(operation,"CREATE");break;
        case 'l': strcpy(operation,"LOCK");break;
        case 'c': strcpy(operation,"REMOVE");break;
        case 'u': strcpy(operation,"UNLOCK");break;
        case 'e': strcpy(operation,"CLOSE");break;
        case 'r': strcpy(operation,"READ");break;
        case 'R': strcpy(operation,"READ n");break;
        case 'w': strcpy(operation,"WRITE n");break;
        case 'W': strcpy(operation,"WRITE");break;
        default: strcpy(operation,"UNKNOWN");
    }
}

//---------------------------------------------------------------------------------
static char *esitoToStr(int esito) {
    switch (esito) {
        case ERR_NO_ERROR:
            return "ERR_NO_ERROR";
        case ERR_CONNECTION_NOT_OPEN :
            return "ERR_CONNECTION_NOT_OPEN";
        case ERR_CONNECTION_NOT_CLOSED:
            return "ERR_CONNECTION_NOT_CLOSED";
        case ERR_RECEIVE_DATA:
            return "ERR_RECEIVE_DATA";
        case ERR_OUT_OF_MEMORY:
            return "ERR_OUT_OF_MEMORY";
        case ERR_SEND_DATA:
            return "ERR_SEND_DATA";
        case ERR_DIR_NOT_SPECIFIED:
            return "ERR_DIR_NOT_SPECIFIED";
        case ERR_INVALID_NUMBER_OF_ARGUMENTS:
            return "ERR_INVALID_NUMBER_OF_ARGUMENTS";
        case ERR_DIRECTORY_NOT_FOUND:
            return "ERR_DIRECTORY_NOT_FOUND";
        case ERR_OPEN_FAILURE:
            return "ERR_OPEN_FAILURE";
        case ERR_READ_ERROR:
            return "ERR_READ_ERROR";
        case ERR_FILE_CLOSE_ERROR:
            return "ERR_FILE_CLOSE_ERROR";
        case ERR_LOCK_FILE_FAILURE:
            return "ERR_LOCK_FILE_FAILURE";
        case ERR_UNLOCK_FILE_FAILURE:
            return "ERR_UNLOCK_FILE_FAILURE";
        case ERR_FILE_NOT_REMOVED:
            return "ERR_FILE_NOT_REMOVED";
        default:
            return "UNKNOWN_ERROR";
    }
}

//---------------------------------------------------------------------------------
void reportOperation(int op, char *arguments, int readBytes, int writtenBytes, int esito) {
    char operation[40];
    opToStr(op,operation);
    if (readBytes >= 0)
        printf("%s: %s  read bytes = %d, (result:%d %s)\n", operation, arguments, readBytes, esito, esitoToStr(esito));
    else if (writtenBytes >= 0)
        printf("%s: %s  written bytes = %d, (result:%d %s)\n", operation, arguments, writtenBytes, esito, esitoToStr(esito));
    else
        printf("%s: %s  (result:%d %s)\n", operation, arguments, esito, esitoToStr(esito));
}

//---------------------------------------------------------------------------------
void logOperazionePreExec(int op, char * arguments){
    char operation[40];
    opToStr(op,operation);
    logMsgf("Prepare to execute -  %s: %s", operation, arguments);
}

//---------------------------------------------------------------------------------
void logOperazionePostExec(int op, char * arguments, int readBytes, int writtenBytes, int esito){
    char operation[40];
    opToStr(op,operation);
    if (readBytes >= 0)
        logMsgf("Executed - %s: %s  read bytes = %d, (result:%d %s)", operation, arguments, readBytes, esito, esitoToStr(esito));
    else if (writtenBytes >= 0)
        logMsgf("Executed - %s: %s  written bytes = %d, (result:%d %s)", operation, arguments, writtenBytes, esito, esitoToStr(esito));
    else
        logMsgf("Executed - %s: %s  (result:%d %s)", operation, arguments, esito, esitoToStr(esito));
}

