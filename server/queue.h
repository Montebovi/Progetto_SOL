
#ifndef PROGETTO_MICHELE_QUEUE_H
#define PROGETTO_MICHELE_QUEUE_H

#include <stddef.h>
#include <pthread.h>

typedef enum queue_ret
{
    QUEUE_RET_Success,
    QUEUE_RET_FailedMemoryAlloc,
    QUEUE_RET_QueueIsEmpty,
} queue_ret_t;

typedef struct node
{
    void* data;
    struct node* next;
} node_t;

typedef struct queue
{
    node_t* front;
    node_t* back;
    size_t size;
    pthread_mutex_t mutex;
} queue_t;

queue_ret_t QUEUE_initialize(queue_t** queue);
size_t QUEUE_size(queue_t* queue);
queue_ret_t QUEUE_enqueue(queue_t* queue, void* data);
queue_ret_t QUEUE_peek(queue_t* queue, void* data, size_t size);
queue_ret_t QUEUE_dequeue(queue_t* queue, void* data, size_t size);
void QUEUE_free(queue_t* queue);

#endif //PROGETTO_MICHELE_QUEUE_H
