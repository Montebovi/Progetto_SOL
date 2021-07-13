#ifndef CLIENT_OPZIONIMAN_H
#define CLIENT_OPZIONIMAN_H

typedef struct{
    int opzione;
    char *arg;
} opzione_t;

#define MAX_OPTS   100

struct optionsType {
    int help;                         //-h
    char * socketName;                //-f
    char * directoryScritturaFile;    //-d
    char * directoryMemSecondariaWhenCapacityMisses; // -D cartella in memoria secondaria dove vengono scritti
                                                    // (lato client) i file che il server rimuove a seguito di capacity misses
    int tempoRitardoMinimoMs;
    int abilitaStampe;
    opzione_t opzioni[MAX_OPTS];
};

void freeOpzioni(struct optionsType opzioni);  // free memoria opzioni
void showOpzioni(struct optionsType opzioni);  // mostra le opzioni impostate

void print_client_opt();

int caricaOpzioni(int argc, char **pString, struct optionsType *pType);

#endif //CLIENT_OPZIONIMAN_H
