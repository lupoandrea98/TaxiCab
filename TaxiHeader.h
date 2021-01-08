#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define EXIT_ON_ERROR if (errno) {fprintf(stderr,  \
              "file: %s; %d: pid %ld; errno: %d (%s)\n",     \
              __FILE__,                            \
              __LINE__,                            \
              (long) getpid(),                     \
              errno,                               \
              strerror(errno)); exit(EXIT_FAILURE);}

//per i font della stampa:   
#define BOLDRED "\033[1m\033[31m"   
#define GREEN   "\033[32m"
#define CYAN    "\033[36m" 
#define RESET   "\033[0m"  //per resettare il font dei caratteri

#define MSGKEY 151297 //per la coda di messaggi
#define SHMKEY 130198 //per la memoria condivisa 

#define MSIZE 256

//da ricompilare a tempo di esecuzione!------------------------------------
#define SO_HEIGHT 6 //RIGHE
#define SO_WIDTH 4 //COLONNE
//-------------------------------------------------------------------------

int SO_HOLES; //numero celle inaccessibili (<= W x H)
long SO_TIMEOUT; //tempo di vita del taxi 
unsigned int SO_DURATION; //durata di simulazione
int SO_TOP_CELLS; //Valore che deve contenere il massimo di numVolteAttr presente nella struct cella.
int SO_SOURCES; //num celle sorgenti (<= W x H - HOLES )
int SO_TAXI;
int SO_CAP_MIN; //ci servirà per determinare la capMax
int SO_CAP_MAX; //ci servirà per determinare la capMax
unsigned long SO_TIMENSEC_MIN; //ci servirà per determinare tempAttravers (nanosec) per ogni cella
unsigned long SO_TIMENSEC_MAX; //ci servirà per determinare tempAttravers (nanosec) per ogni cella

struct cella{
    int tipo; //0 libera   1 buco   2 sorgente  3taxi   4 taxi+sorgente
    char carattere; //x fare un disegno nella stampa? '0'vuota  'X'occupate  'S'sorgenti   '*'taxi 
    int cellaCoord[2]; //cordinate della cella. coord[0] è la riga coord[1] è la colonna        
    int numTaxiPresenti; //numero compreso tra SO_CAP_MIN e SO_CAP_MAX che indica quanti taxi ci sono su quella cella... QUINDI I TAXI POSSONO SOVRAPPORSI!!
    int numVolteAttr; //viene incrementato se è stato attraversato da un taxi
    struct timespec tempAttravers; //un numero random tra SO_TIMENSEC_MIN e SO_TIMENSEC_MAX nanosecondi (ABBIAMO BISOGNO DELLA STRUCT).
    //tempoAttravers.tv_sec ---> secondi
    //tempoAttravers.tv_nsec ---> nanosecondi
    //questa struct serve ad usare correttamente la nano sleep.
    int capMax; //Capacità massima della cella
};

struct taxi{
    long pid; //il pid del processo taxi
    int percorso; //si incrementa per ogni cella che attraversa
    int viaggioPiuLungo; //tiene traccia del viaggio più lungo che ha fatto
    int richiesteRacc; //numero delle richieste che ha raccolto
    int coordTaxi[2]; //le coordinate del taxi
    long tempoImpiegato;
};

struct source{
    long pid; //il pid del processo taxi
    int coordSource[2]; //le coordinate del taxi
};

//per la coda di messaggi
struct queue{      
    long mtype;  //tipo del messaggio
    char msg[MSIZE]; //contenuto del messaggio
    int arrivo[2]; //le coordinate di dove si vuole arrivare
    int partenza[2]; //coordinate di partenza
       
};

//per la memoria condivisa
struct data{
   struct cella mappa[SO_HEIGHT][SO_WIDTH];
   //da rivedere
   int numTaxi; //il numero dei taxi (come SO_TAXI)
   int numSourc; //il numero dei taxi (come SO_SOURCES)
};

//Ho bisogno di definire questa struct perchè su linux al contrario di macOS non basta importare la libreria dei semafori.
union semun {
        int              val;    // Value for SETVAL 
        struct semid_ds *buf;    // Buffer for IPC_STAT, IPC_SET 
        unsigned short  *array;  // Array for GETALL, SETALL 
        struct seminfo  *__buf;  // Buffer for IPC_INFO (Linux-specific) 
};


//==============================================================================================================
//                                           PROTOTIPI DI FUNZIONI
//==============================================================================================================

//Funzioni per la generazione:
void mapGenerator(struct cella map[SO_HEIGHT][SO_WIDTH]);

void HolesGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]);

void SourcesGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]);

void TaxiGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]);


//funzioni per la ricerca:
void cercaTaxi(struct taxi *st, struct cella A[SO_HEIGHT][SO_WIDTH]);

void cercaSource(struct source *st, struct cella A[SO_HEIGHT][SO_WIDTH]);

int controlloBuchi(struct cella A[SO_HEIGHT][SO_WIDTH], int i, int j);

void getCellaArrivo(struct cella A[SO_HEIGHT][SO_WIDTH], struct queue *coda);


//varie:
int TaxiMover(struct cella mappa[SO_HEIGHT][SO_WIDTH], struct taxi *st, int arrivo_r, int arrivo_c, long *tempoImpiegato); //algoritmo movimento del taxi

void rispondi(const struct queue* richiesta, int msgid, int valid); //per la risposta nella coda dei messaggi

void printMap(struct cella map[SO_HEIGHT][SO_WIDTH]); //per la stampa della mappa


//per i semafori:
int initSemAvailable(int semId, int semNum);
int initSemInUse(int semId, int semNum);
int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);

