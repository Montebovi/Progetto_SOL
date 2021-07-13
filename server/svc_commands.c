#include <stdlib.h>
#include <string.h>
#include "../common/svc_commands.h"
#include "svc_request.h"

void freeCommand(commandToSvr_t *cmd){
    if (cmd->pathname)
        free(cmd->pathname);
    if (cmd->dirname)
        free(cmd->dirname);
    if (cmd->data)
        free(cmd->data);
    free(cmd);
}

//--------------------------------------------------------------------------------------------------
void cmdToStr(int cmd, char *str){
    switch (cmd) {
        case CMD_EOF:
            strcpy(str,"CMD_EOF");
            break;
        case CMD_OPEN_FILE:
            strcpy(str,"CMD_OPEN_FILE");
            break;
        case CMD_READ_FILE:
            strcpy(str,"CMD_READ_FILE");
            break;
        case CMD_WRITE_FILE:
            strcpy(str,"CMD_WRITE_FILE");
            break;
        case CMD_APPEND_TO_FILE:
            strcpy(str,"CMD_APPEND_TO_FILE");
            break;
        case CMD_LOCK_FILE:
            strcpy(str,"CMD_LOCK_FILE");
            break;
        case CMD_UNLOCK_FILE:
            strcpy(str,"CMD_UNLOCK_FILE");
            break;
        case CMD_CLOSE_FILE:
            strcpy(str,"CMD_CLOSE_FILE");
            break;
        case CMD_REMOVE_FILE:
            strcpy(str,"CMD_REMOVE_FILE");
            break;
        case CMD_READ_N_FILE:
            strcpy(str,"CMD_READ_N_FILE");
            break;
        default:
            strcpy(str,"UNKOWN CMD");
            break;
    }
}