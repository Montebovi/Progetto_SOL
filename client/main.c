#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>

#include "fileutilities.h"
#include "opzioniman.h"

#include "conn.h"
#include "util.h"
#include "server_api.h"
#include "errors.h"
#include "optionsExec.h"
#include "operationsReport.h"
#include "logFile.h"
#include "../common/svc_commands.h"


void showData(void *data, int sizeOfData, int maxNum){
    int n = maxNum;
    if (n > sizeOfData)
        n = sizeOfData;
    unsigned char * bytes = data;
    printf("Primi %d bytes di dati: ",n);
    for (int i=0; i < n; i++) {
        printf("%hhx - ",bytes[i]);
    }
    printf("\n");
}

// gcc -Wall -pedantic -g main.c fileutilities.c  -o main
// valgrind --leak-check=full ./main

// cat arguments.dat | xargs ./myprogram
//  xargs -a test1.txt ./client
int main(int argc, char *argv[]) {

    logOpenFile("clientLog.txt",0);

    logMsgf( "---------------------- Avvio client ----------------------");

    for (int i = 0; i <argc; ++i) {
        printf("%d) %s\n",i,argv[i]);
    }
    printf("**********************************************\n");

    struct optionsType opzioni;

    // caricamento opzioni con significato trasversale a prescindere dall'ordine di specifica
    int res = caricaOpzioni(argc,argv, &opzioni);
    if (res == -1)
    {
        logMsgf( "Opzioni non valide.");

        printf("Opzioni non valide. Usare -h per mostrare elenco comandi gestiti.\n");
        logCloseFile();
        return 0;
    }

    if (opzioni.directoryScritturaFile != NULL && !directoryExists(opzioni.directoryScritturaFile)){
        printf("Directory [%s] non trovata.\n",opzioni.directoryScritturaFile);
        logCloseFile();
        return 0;
    }

    if (opzioni.directoryMemSecondariaWhenCapacityMisses != NULL && !directoryExists(opzioni.directoryMemSecondariaWhenCapacityMisses)){
        printf("Directory [%s] non trovata.\n",opzioni.directoryMemSecondariaWhenCapacityMisses);
        logCloseFile();
        return 0;
    }

    // caso di -h
    if (opzioni.help)
    {
        logCloseFile();
        print_client_opt();
        exit(0);
    }

    showOpzioni(opzioni);

    struct timespec expiryTime;
    time ( &expiryTime.tv_sec );
    expiryTime.tv_sec += 120;
    res = openConnection(opzioni.socketName,1000,expiryTime);
    if (res==-1)
    {
        printf("Connessione fallita\n");
    }

    errno =0;
    int k=0;
    int retVal;
    while (opzioni.opzioni[k].opzione  && k <MAX_OPTS  ){
        int numReadBytes = -1;
        int numWrittenBytes = -1;
        char * arg = strdup(opzioni.opzioni[k].arg);
        logOperazionePreExec(opzioni.opzioni[k].opzione, arg);
        switch (opzioni.opzioni[k].opzione) {
            // nuova opzione: -o file1[,file2]: lista di nomi di file da aprire nel server separati da ‘,’;
            case 'o':{
                retVal = execute_open(opzioni.opzioni[k].arg, 0,opzioni);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // nuova opzione: -b file1[,file2]: lista di nomi di file da aprire in creazione nel server separati da ‘,’;
            case 'b':{
                retVal = execute_open(opzioni.opzioni[k].arg, O_CREATE,opzioni);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // lock uno o più files
            case 'l':{
                retVal = execute_lock(opzioni.opzioni[k].arg);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // remove uno o più files
            case 'c':{
                retVal = execute_remove(opzioni.opzioni[k].arg);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // unlock uno o più files
            case 'u':{
                retVal = execute_unlock(opzioni.opzioni[k].arg);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // nuova opzione: -e file1[,file2]: lista di nomi di file da chiudere nel server separati da ‘,’;
            case 'e':{
                retVal = execute_close(opzioni.opzioni[k].arg);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // lettura uno o più files
            case 'r':{
                void *data;
                retVal = execute_r(opzioni.opzioni[k].arg,opzioni, &data, &numReadBytes);
                if (retVal != 0)
                    manageRetValError(retVal);
                if (numReadBytes > 0){
                    showData(data, numReadBytes, 16);
                    free(data);
                    data = NULL;
                }
                break;
            }
            // lettura di N files
            case 'R':{
                retVal = execute_readN(opzioni.opzioni[k].arg,opzioni);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // scrittura di N files da directory
            case 'w':{
                retVal = execute_w(opzioni.opzioni[k].arg,opzioni,&numWrittenBytes);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
            // scrittura di uno o più files
            case 'W':{
                 retVal = execute_W(opzioni.opzioni[k].arg,opzioni,&numWrittenBytes);
                if (retVal != 0)
                    manageRetValError(retVal);
                break;
            }
        }
        logOperazionePostExec(opzioni.opzioni[k].opzione, arg, numReadBytes, numWrittenBytes, errno);
        if (opzioni.abilitaStampe) {
            reportOperation(opzioni.opzioni[k].opzione, arg, numReadBytes, numWrittenBytes, errno);
            errno = 0;
        }
        free(arg);
        if (opzioni.tempoRitardoMinimoMs > 0)
          usleep(opzioni.tempoRitardoMinimoMs*1000);
        k++;
    }
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> close connection: %s\n",opzioni.socketName);
    res = closeConnection(opzioni.socketName);
    if (res == -1){
        logMsgf( "Errore Chiusura Connessione",argc);
        manageRetValError(retVal);
    }
    else
        logMsgf( "Chiusura Connessione",argc);

    freeOpzioni(opzioni);
    logMsgf( "---------------------- Terminazione Client ----------------------",argc);
    logCloseFile();

    return 0;
}


