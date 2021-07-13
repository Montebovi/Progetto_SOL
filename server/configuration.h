#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_


struct serverConfiguration {
  unsigned long threadsWorkers;
  unsigned long maxSize;
  unsigned long maxFiles;
  char *sockname;
};

void showConfig(struct serverConfiguration configuration);
int readConfig(char *fileName, struct serverConfiguration *config);
void freeConfiguration(struct serverConfiguration configuration);

#endif /* CONFIGURATION_H_ */