#ifndef CLIENT_SVC_COMMANDS_H
#define CLIENT_SVC_COMMANDS_H

#define CMD_EOF             0
#define CMD_OPEN_FILE       1
#define CMD_READ_FILE       2
#define CMD_WRITE_FILE      3
#define CMD_APPEND_TO_FILE  4
#define CMD_LOCK_FILE       5
#define CMD_UNLOCK_FILE     6
#define CMD_CLOSE_FILE      7
#define CMD_REMOVE_FILE     8
#define CMD_READ_N_FILE     9

#define O_CREATE (0x01)
#define O_LOCK   (0x02)

typedef struct {
    int cmd;
    int flags;
    int sizeData;
    int numFiles;
    char * pathname;
    char * dirname;
    void * data;
} commandToSvr_t;

void freeCommand(commandToSvr_t *);

#endif //CLIENT_SVC_COMMANDS_H
