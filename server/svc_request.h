#ifndef PROGETTO_MICHELE_SVC_REQUEST_H
#define PROGETTO_MICHELE_SVC_REQUEST_H

#include "../common/svc_commands.h"

typedef struct {
    commandToSvr_t *cmdDescr;
    int connDescr;
} request_t;

void freeRequest(request_t *req);


#endif //PROGETTO_MICHELE_SVC_REQUEST_H
