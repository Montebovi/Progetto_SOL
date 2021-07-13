#include <stdlib.h>
#include <string.h>

#include "queue.h"

//---------------------------------------------------------------------------------------
queue_ret_t QUEUE_initialize(queue_t** queue)
{
    *queue = (queue_t*)malloc(sizeof(queue_t));

    if ((*queue) == NULL)
        return QUEUE_RET_FailedMemoryAlloc;

    (*queue)->front = NULL;
    (*queue)->back = NULL;
    (*queue)->size = 0;

    pthread_mutex_init(&((*queue)->mutex), NULL);

    return QUEUE_RET_Success;
}

//---------------------------------------------------------------------------------------
size_t QUEUE_size(queue_t* queue)
{
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    return size;
}

//---------------------------------------------------------------------------------------
queue_ret_t QUEUE_enqueue(queue_t* queue, void* data)
{
    pthread_mutex_lock(&queue->mutex);
    node_t* node = (node_t*)calloc(1, sizeof(node_t));
    if (node == NULL)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_RET_FailedMemoryAlloc;
    }

    node->data = data;
    node->next = NULL;

    if (queue->size == 0)
    {
        queue->front = node;
        queue->back = node;
    }
    else
    {
        queue->back->next = node;
        queue->back = node;
    }

    (queue->size)++;

    pthread_mutex_unlock(&queue->mutex);
    return QUEUE_RET_Success;
}

//---------------------------------------------------------------------------------------
queue_ret_t QUEUE_peek(queue_t* queue, void* data, size_t size)
{
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_RET_QueueIsEmpty;
    }

    memcpy(data, queue->front->data, size);

    pthread_mutex_unlock(&queue->mutex);
    return QUEUE_RET_Success;
}

//---------------------------------------------------------------------------------------
void freeNode(node_t* node){
    if (node->data){
        free(node->data);
        node->data = NULL;
    }
    free(node);
    node = NULL;
}

//---------------------------------------------------------------------------------------
queue_ret_t QUEUE_dequeue(queue_t* queue, void* data, size_t size)
{
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_RET_QueueIsEmpty;
    }
    memcpy(data, queue->front->data, size);

    if (queue->front == queue->back)
    {
        freeNode(queue->front);
        queue->front = NULL;
        queue->back = NULL;
    }
    else
    {
        node_t* temp = queue->front;
        queue->front = temp->next;
        freeNode(temp);
    }

    (queue->size)--;

    pthread_mutex_unlock(&queue->mutex);
    return QUEUE_RET_Success;
}

//---------------------------------------------------------------------------------------
void QUEUE_free(queue_t* queue)
{
    pthread_mutex_lock(&queue->mutex);

    node_t* current = queue->front;

    while (current != NULL)
    {
        node_t* next = current->next;
        freeNode(current);
        current = next;
    }

    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}