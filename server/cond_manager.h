#ifndef PROGETTO_MICHELE_COND_MANAGER_H
#define PROGETTO_MICHELE_COND_MANAGER_H

void cond_init();
void cond_deinit();

void cond_signal(char *keyname);
void cond_wait(char *keyname);

#endif //PROGETTO_MICHELE_COND_MANAGER_H
