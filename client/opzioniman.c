#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "opzioniman.h"


//-----------------------------------------------------------------
void freeOpzioni(struct optionsType opzioni) {
    for (int i = 0; i < MAX_OPTS; i++)
        if (opzioni.opzioni[i].arg) {
            free(opzioni.opzioni[i].arg);
            opzioni.opzioni[i].arg = NULL;
        }
}


//*********************************************************************************
// mostra le opzioni
void showOpzioni(struct optionsType opts) {
    printf("%s\n", opts.help ? "help abilitato" : "help non abilitato");
    printf("%s\n", opts.abilitaStampe ? "stampe abilitate" : "stampe non abilitate");
    printf("nome socket: %s\n", opts.socketName ? opts.socketName : "<non specificato>");
    printf("tempo ritardo=%d ms\n", opts.tempoRitardoMinimoMs);
    printf("cartella in memoria secondaria quando capacità non sufficiente: %s\n",
           opts.directoryMemSecondariaWhenCapacityMisses
           ? opts.directoryMemSecondariaWhenCapacityMisses : "<non specificata>");
    printf("cartella in memoria secondaria dove scrivere: %s\n",
           opts.directoryScritturaFile ? opts.directoryScritturaFile : "<non specificata>");
}

//*********************************************************************************
void print_client_opt() {
    char *help[][2] = {
            {"-h",               "visualizza help delle opzioni;"},
            {"-f <filename>",    "specifica il nome del socket AF_UNIX a cui connettersi;"},
            {"-w dirname[,n=0]", "invia al server i file nella cartella ‘dirname’, ovvero effettua una richiesta di scrittura al\n"
                                 "server per i file. Se la directory ‘dirname’ contiene altre directory, queste vengono visitate ricorsivamente\n"
                                 "fino a quando non si leggono ‘n‘ file; se n=0 (o non è specificato) non c’è un limite superiore al numero di\n"
                                 "file da inviare al server (tuttavia non è detto che il server possa scriverli tutti).;"},
            {"-W file1[,file2]", "lista di nomi di file da scrivere nel server separati da ‘,’;"},
            {"D dirname",        " cartella in memoria secondaria dove vengono scritti (lato client) i file che il server rimuove a\n"
                                 "seguito di capacity misses (e che erano stati modificati da operazioni di scrittura) per servire le scritture\n"
                                 "richieste attraverso l’opzione ‘-w’ e ‘-W’. L’opzione ‘-D’ deve essere usata quindi congiuntamente\n"
                                 "all’opzione ‘-w’ o ‘-W’, altrimenti viene generato un errore. Se l’opzione ‘-D’ non viene specificata, tutti i\n"
                                 "file che il server invia verso il client a seguito di espulsioni dalla cache, vengono buttati via. Ad esempio,\n"
                                 "supponiamo i seguenti argomenti a linea di comando “-w send -D store”, e supponiamo che dentro la cartella\n"
                                 "‘send’ ci sia solo il file ‘pippo’. Infine supponiamo che il server, per poter scrivere nello storage il file\n"
                                 "‘pippo’ deve espellere il file ‘pluto’ e ‘minni’. Allora, al termine dell’operazione di scrittura, la cartella\n"
                                 "‘store’ conterrà sia il file ‘pluto’ che il file ‘minni’. Se l’opzione ‘-D’ non viene specificata, allora il server\n"
                                 "invia sempre i file ‘pluto’ e ‘minni’ al client, ma questi vengono buttati via;"},
            {"-r file1[,file2]", "lista di nomi di file da leggere dal server separati da ‘,’ (esempio: -r pippo,pluto,minni);"},
            {"R [n=0]",          "tale opzione permette di leggere ‘n’ file qualsiasi attualmente memorizzati nel server; se n=0 (o\n"
                                 "non è specificato) allora vengono letti tutti i file presenti nel server;"},
            {"-d dirname",       "cartella in memoria secondaria dove scrivere i file letti dal server con l’opzione ‘-r’ o ‘-R’.\n"
                                 "L’opzione -d va usata congiuntamente a ‘-r’ o ‘-R’, altrimenti viene generato un errore; Se si utilizzano le\n"
                                 "opzioni ‘-r’ o ‘-R’senza specificare l’opzione ‘-d’ i file letti non vengono memorizzati sul disco;"},
            {"-t time",          "tempo in millisecondi che intercorre tra l’invio di due richieste successive al server (se non\n"
                                 "specificata si suppone -t 0, cioè non c’è alcun ritardo tra l’invio di due richieste consecutive);"},
            {"-l file1[,file2]", " lista di nomi di file su cui acquisire la mutua esclusione;"},
            {"-u file1[,file2]", "lista di nomi di file su cui rilasciare la mutua esclusione;"},
            {"-c file1[,file2]", "lista di file da rimuovere dal server se presenti;"},
            {"-p",               "abilita le stampe sullo standard output per ogni operazione. Le stampe associate alle varie operazioni\n"
                                 "riportano almeno le seguenti informazioni: tipo di operazione, file di riferimento, esito e dove è rilevante i\n"
                                 "bytes letti o scritti."},
    };
    int size = sizeof help / sizeof help[0];
    printf("Uso del programma: client <opzioni>\n");
    for (int i = 0; i < size; i++)
        printf("%s: %s\n", help[i][0], help[i][1]);

}

//*********************************************************************************
int caricaOpzioni(int argc, char *argv[], struct optionsType *opzioni) {
    opzioni->help = 0;
    opzioni->abilitaStampe = 0;
    opzioni->tempoRitardoMinimoMs = 0;
    opzioni->socketName = NULL;
    opzioni->directoryScritturaFile = NULL;
    opzioni->directoryMemSecondariaWhenCapacityMisses = NULL;

    for (int i = 0; i < MAX_OPTS; i++) {
        opzioni->opzioni[i].arg = 0;
        opzioni->opzioni[i].opzione = 0;
    }

    int k = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":phf:w:W:D:R:r:d:t:l:u:c:b:o:e:")) != -1) {
        switch (opt) {
            case 'b':
            case 'o':
            case 'e':
            case 'w':
            case 'W':
            case 'r':
            case 'R':
            case 'l':
            case 'u':
            case 'c':
                opzioni->opzioni[k].opzione = opt;
                if (optarg != NULL)
                    opzioni->opzioni[k].arg = strdup(optarg);
                else
                    opzioni->opzioni[k].arg = NULL;
                k++;
                break;
            case 'h':
                if (opzioni->help == 1)
                    return -1;
                opzioni->help = 1;
                break;
            case 'f':  // specifica il nome del socket AF_UNIX a cui connettersi;
                if (opzioni->socketName != NULL)
                    return -1;
                opzioni->socketName = optarg;
                break;
            case 'D': // cartella in memoria secondaria quando manca capacità
                opzioni->directoryMemSecondariaWhenCapacityMisses = optarg;
                break;
            case 'd': //  cartella in memoria secondaria dove scrivere i file letti dal server con l’opzione ‘-r’ o ‘-R’
                opzioni->directoryScritturaFile = optarg;
                break;
            case 't': //tempo in millisecondi che intercorre tra l’invio di due richieste successive al server
                opzioni->tempoRitardoMinimoMs = atoi(optarg);
                break;
            case 'p':
                if (opzioni->abilitaStampe == 1)
                    return -1;
                opzioni->abilitaStampe = 1;
                break;
            default:
                break;
        }
    }
    return 0;
}


