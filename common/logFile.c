#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "logFile.h"

static FILE *fp;

int logOpenFile(char *logFileName, int reset){
    if (reset)
      fp  = fopen (logFileName, "w");
    else
      fp  = fopen (logFileName, "a");
    return 0;
}

void logMsgf(const char *fmt, ...){
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logMsg(buffer);
}

void logMsg(char *msg){
    char text[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(text, sizeof(text)-1, "%d %m %Y %H:%M:%S", t);
    fprintf(fp,"[%s] - %s\n",text,msg);
    fflush(fp);
}

int logCloseFile(){
    fclose(fp);
    return 0;
}
