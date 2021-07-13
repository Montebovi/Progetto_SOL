#ifndef CLIENT_LOGFILE_H
#define CLIENT_LOGFILE_H

int logOpenFile(char *logFileName, int reset);
void logMsg(char *msg);
void logMsgf(const char *fmt, ...);
int logCloseFile();

#endif //CLIENT_LOGFILE_H
