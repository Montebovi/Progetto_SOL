#ifndef CLIENT_OPERATIONSREPORT_H
#define CLIENT_OPERATIONSREPORT_H

#include <stdbool.h>

void reportOperation(int op, char * arguments, int readBytes, int writtenBytes, int esito);

void logOperazionePreExec(int op, char * arguments);
void logOperazionePostExec(int op, char * arguments, int readBytes, int writtenBytes, int esito);

#endif //CLIENT_OPERATIONSREPORT_H
