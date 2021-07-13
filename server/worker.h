#ifndef PROGETTO_MICHELE_WORKER_H
#define PROGETTO_MICHELE_WORKER_H

#include "queue.h"

typedef struct {
    int threadId;
    queue_t * queue;
} workerParams_t;

void * handle_client_request(void */*workerParams_t **/ workerParams);
void endWorkers();


#endif //PROGETTO_MICHELE_WORKER_H
