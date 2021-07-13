#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "../common/logFile.h"
#include "cond_manager.h"

static pthread_cond_t cv;
static pthread_mutex_t lock;

//------------------------------------------------------------------------------------
void cond_init()
{
}

//------------------------------------------------------------------------------------
void cond_deinit()
{
}

//------------------------------------------------------------------------------------
void cond_signal(char *keyname){
    pthread_mutex_lock(&lock);
    logMsgf("SIGNAL: %s",keyname);
    printf("SIGNAL: %s\n",keyname);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lock);
}

//------------------------------------------------------------------------------------
void cond_wait(char *keyname){
    pthread_mutex_lock(&lock);
    logMsgf("WAIT: %s",keyname);
    printf("WAIT: %s\n",keyname);
    pthread_cond_wait(&cv, &lock);
    logMsgf("WAKEUP: %s",keyname);
    printf("WAKEUP: %s\n",keyname);
    pthread_mutex_unlock(&lock);
}
