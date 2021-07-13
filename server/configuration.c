#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/logFile.h"
#include "configuration.h"

//------------------------------------------------------------------------------------
void showConfig(struct serverConfiguration configuration) {
    printf("Configurazioni lette:\n");
    if (configuration.sockname != NULL)
        printf("%s \t-> [%s]\n", "sockname", configuration.sockname);
    printf("%s \t-> [%lu]\n", "maxFiles", configuration.maxFiles);
    printf("%s \t-> [%lu]\n", "maxSize", configuration.maxSize);
    printf("%s \t-> [%lu]\n", "threadsWorkers", configuration.threadsWorkers);

    logMsgf("Configurazione sockname:    [%s]",configuration.sockname);
    logMsgf("Configurazione maxFiles:    [%lu]",configuration.maxFiles);
    logMsgf("Configurazione maxSize:     [%lu]", configuration.maxSize);
    logMsgf("Configurazione num workers: [%lu]", configuration.threadsWorkers);
}

//------------------------------------------------------------------------------------
void freeConfiguration(struct serverConfiguration configuration){
    free(configuration.sockname);
    configuration.sockname = NULL;
}


//------------------------------------------------------------------------------------
int readConfig(char *fileName, struct serverConfiguration *config) {
    // Apro il file e guardo la dimensione
    FILE *fd;
    fd = fopen(fileName, "r");
    if( fd == NULL ) return -1;
    fseek(fd, 0L, SEEK_END);
    int fileSize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *currLine = (char *)malloc(fileSize * sizeof(char));
    char *key = (char *)malloc(fileSize * sizeof(char));
    char *value = (char *)malloc(fileSize * sizeof(char));

    int up = 0;
    // Finchè ho righe da leggere
    while (fgets(currLine, fileSize, fd) != NULL) {
        int lineSize = strlen(currLine);

        // se vuota o commento salto
        if (lineSize <= 1 || currLine[0] == '#') continue;

        // leggo chiave e valore
        sscanf(currLine, "%s = %s", key, value);

        int keySize = strlen(key);
        int valueSize = strlen(value);
        if (keySize == 0 || valueSize == 0) continue;

        // controllo a quale parametro la chiave si riferisce
        if (!up && strcmp(key, "sockname") == 0) {
            up = 1;
            config->sockname = (char *)malloc((valueSize + 1) * sizeof(char));
#if defined(MAKE_VALGRIND_HAPPY)
            memset(config->unixPath, 0, (valueSize + 1) * sizeof(char));
#endif
            strncpy(config->sockname, value, valueSize + 1);
        } else if (strcmp(key, "maxFiles") == 0) {
            config->maxFiles = strtoul(value, NULL, 10);
        } else if (strcmp(key, "threadsWorkers") == 0) {
            config->threadsWorkers = strtoul(value, NULL, 10);
        } else if (strcmp(key, "maxSize") == 0) {
            config->maxSize = strtoul(value, NULL, 10);
        }

        fflush(stdout);
    }

    // Libero la memoria alloca e chiudo il file aperto
    free(currLine);
    currLine = NULL;
    free(key);
    key = NULL;
    free(value);
    value = NULL;

    fclose(fd);
    return 0;
}

