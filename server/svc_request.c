#include "stdlib.h"
#include "svc_request.h"

void freeRequest(request_t *req){
    if (req->cmdDescr){
        freeCommand(req->cmdDescr);
        req->cmdDescr = NULL;
    }
    free(req);
}