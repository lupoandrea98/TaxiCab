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
#define RESET   "\033[0m"  //per resettare il font dei caratteri
#define CYAN    "\033[36m"  

//da ricompilare se cambiate:
#define SO_HEIGHT 7 //RIGHE
#define SO_WIDTH 7 //COLONNE

#define MSGKEY 151297 //per la coda di messaggi
#define SHMKEY 73459 //per la memoria condivisa  METTERE LA DATA DI ANDRE!

#define MSIZE 256
//runtime:
int SO_HOLES = 0; //numero celle inaccessibili (<= W x H)
int SO_SOURCES = 0; //num celle origine taxi (<= W x H - HOLES )
int SO_TAXI = 0;
long SO_TIMEOUT; //tempo massimo di risposta del taxi
unsigned int SO_DURATION; //durata di simulazione
int SO_TOP_CELLS = 0; //Valore che deve contenere il massimo di numVolteAttr presente nella struct cella.

/*LE INIZIALIZZO NELLA FUNZIONE MAPGENERATOR

unsigned int SO_TIMENSEC_MIN; //tempo minimo attraversamento cella (nanosec)
unsigned int SO_TIMENSEC_MAX; //tempo massimo attraversamento cella (nanosec)
unsigned int SO_CAP_MIN; //num minimo taxi ospitati in una cella
unsigned int SO_CAP_MAX; //num massimo taxi ospitati in una cella
*/

struct cella{
    int tipo; //0 libera   1 buco   2 sorgente  3taxi   4 taxi+sorgente
    char carattere; //x fare un disegno nella stampa? '0'vuota  'X'occupate  'S'sorgenti   '*'taxi 
    int cellaCoord[2]; //cordinate della cella. coord[0] è la riga coord[1] è la colonna        
    int numTaxiPresenti; //numero compreso tra SO_CAP_MIN e SO_CAP_MAX che indica quanti taxi ci sono su quella cella... QUINDI I TAXI POSSONO SOVRAPPORSI!!
    struct timespec tempAttravers; //un numero random tra SO_TIMENSEC_MIN e SO_TIMENSEC_MAX nanosecondi (ABBIAMO BISOGNO DELLA STRUCT).
    //tempoAttravers.tv_sec ---> secondi
    //tempoAttravers.tv_nsec ---> nanosecondi
    //questa struct serve ad usare correttamente la nano sleep.
    int numVolteAttr; //viene incrementato se è stato attraversato da un taxi
    int capMax; //Capacità massima della cella
};

struct queue{      
    long mtype;  //tipo del messaggio
    char msg[MSIZE]; //contenuto del messaggio
    int arrivo[2]; //le coordinate di dove si vuole arrivare
    struct cella map[SO_HEIGHT][SO_WIDTH];
    int numTaxi;
    int numSource;
};

//Ho bisogno di definire questa struct perchè su linux al contrario di macOS non basta importare la libreria dei semafori.
union semun {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

struct taxi{
    long pid; //il pid del processo taxi
    int percorso; //si incrementa per ogni cella che attraversa
    int viaggioPiuLungo; //tiene traccia del viaggio più lungo che ha fatto
    int richiesteRacc; //numero delle richieste che ha raccolto
    int coordTaxi[2]; //le coordinate del taxi
};


//====================================================================
//                      FUNZIONI PER SEMAFORI
//====================================================================

// Initialize semaphore to 1 (i.e., "available")
int initSemAvailable(int semId, int semNum) {
	union semun arg;
	arg.val = 1;

	return semctl(semId, semNum, SETVAL, arg);
}

// Initialize semaphore to 0 (i.e., "in use")
int initSemInUse(int semId, int semNum) {
	union semun arg;
	arg.val = 0;

	return semctl(semId, semNum, SETVAL, arg);
}

// Reserve semaphore - decrement it by 1
int reserveSem(int semId, int semNum) {
	struct sembuf sops;

	sops.sem_num = semNum;
	sops.sem_op = -1;
	sops.sem_flg = 0;

	return semop(semId, &sops, 1);
}

// Release semaphore - increment it by 1
int releaseSem(int semId, int semNum) {
	struct sembuf sops;
	
	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(semId, &sops, 1);
}

