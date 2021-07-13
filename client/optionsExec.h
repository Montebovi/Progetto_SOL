
#ifndef CLIENT_OPTIONSEXEC_H
#define CLIENT_OPTIONSEXEC_H

int execute_open(char *arg, int mode, struct optionsType opzioni);
int execute_close(char *arg);

int execute_remove(char *arg);

int execute_lock(char *arg);
int execute_unlock(char *arg);

int execute_readN(char *arg,struct optionsType opzioni);
int execute_r(char *arg,struct optionsType opzioni, void** data, int* numReadBytes);
int execute_w(char *arg,struct optionsType opzioni,int *numWrittenBytes);
int execute_W(char *arg,struct optionsType opzioni,int *numWrittenBytes);

#endif //CLIENT_OPTIONSEXEC_H
