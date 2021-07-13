#include <stdio.h>
#include "util.h"
#include "../common/logFile.h"
#include "server.h"

// valgrind --leak-check=full  ./progetto_michele

/*
 * valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./progetto_michele <params>
 * */
int main(int argc, char *argv[]) {
    logOpenFile("serverLog.txt",1);
    logMsg("--------------------------------------- AVVIO SERVER --------------------------------------- ");
    run_server(argc,argv);
    logMsg("--------------------------------------- TERMINAZIONE SERVER --------------------------------------- ");
    logCloseFile();
    return 0;
}
