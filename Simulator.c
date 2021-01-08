#include "TaxiFunctions.h"
//PROGRAMMA DI SIMULAZIONE CHE GENERA E INIZIALIZZA LA MAPPA (MESSA IN MEMORIA CONDIVISA), FORKA SO_SOURCES E GESTISCE LE RICHIESTE DI TAXI
//GESTIONE DEI NOSTRI SEGNALI CHE SCRIVONO RICHIESTE NELLA CODA
//GESTIONE DEI SEMAFORI PER LE CELLE OCCUPATE(?)

int codaId, dataId, i;
struct queue coda;
struct data* ptMemCond;
struct source infoSource;


void handler(int sig){

    if((sig == SIGALRM) || (sig == SIGINT)){

        infoSource.pid = (long) getpid();
        cercaSource(&infoSource, ptMemCond->mappa); //Cerco una casella con la sorgente
        coda.partenza[0] = infoSource.coordSource[0];
        coda.partenza[1] = infoSource.coordSource[1];
        coda.mtype = (long) getpid();
        getCellaArrivo(ptMemCond->mappa, &coda); //metodo che mi dà random delle coordinate di arrivo e me le scrive nella coda di messaggi
        printf("arrivo è [%d][%d]\n", coda.arrivo[0], coda.arrivo[1]);
        //invio alla coda
        if(msgsnd(codaId, &coda, sizeof(coda), IPC_NOWAIT)== -1){
            EXIT_ON_ERROR
        }

    }

}

int main(){

    //DA METTERE POI NELLE FUNZIONI DI GENERAZIONE
    SO_HOLES = 0; //numero celle inaccessibili (<= W x H)
    SO_SOURCES = 0; //num celle origine taxi (<= W x H - HOLES )
    SO_TAXI = 0;
    SO_TOP_CELLS = 0; //Valore che deve contenere il massimo di numVolteAttr presente nella struct cella.

    //creazione memoria condivisa
    if((dataId = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*2), IPC_CREAT | 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

    //attacco alla memoria condivisa 
    if((ptMemCond = (struct data*)shmat(dataId, NULL, 0))==(void*)-1){ //attacco segmento
        EXIT_ON_ERROR
    }

    mapGenerator(ptMemCond->mappa);

    HolesGenerator(ptMemCond->mappa);

    SourcesGenerator(ptMemCond->mappa);

    TaxiGenerator(ptMemCond->mappa);

    printMap(ptMemCond->mappa);

    ptMemCond->numTaxi = SO_TAXI;
    ptMemCond->numSourc = SO_SOURCES;

    printf("i buchi sono: %d\n", SO_HOLES);
    printf("le sorgenti sono: %d\n", SO_SOURCES);
    printf("i taxi sono: %d\n", SO_TAXI); //se ne vedi di meno sulla mappa è perché ce ne sono di più in una cella

   //controllo che le sorgenti o i taxi non siano 0
    if((SO_SOURCES == 0) || (SO_TAXI == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n");
      
        exit(EXIT_FAILURE);
    } 

    //si attacca alla coda di messaggi
    if((codaId = msgget(MSGKEY, 0 ))==-1){ //mi aggancio alla coda (0 xk non mi interessano i flag)
        EXIT_ON_ERROR   
    } 
    
    //forko una volta xk 1 SOURCE di prova, ma dovrò forkare SO_SOURCE volte!
    switch(fork()){

        case -1:
            EXIT_ON_ERROR

        case 0: //è il figlio (RICHIESTA TAXI)   
            
            while(SO_DURATION>0){
                alarm(5);
                if(signal(SIGALRM, handler) == SIG_ERR){
                    EXIT_ON_ERROR
                }
                sleep(2);
                //riceve risposta dal taxi
                if(msgrcv(codaId, &coda, sizeof(coda), getpid(), 0) == -1){ 
                    EXIT_ON_ERROR
                }
                printf("Risposta ricevuta! Il figlio del server mi ha scritto '%s'\n", coda.msg);   
                SO_DURATION -= 7;
            }     
               
             //6.Ricevuta la risposta termina.
            exit(EXIT_SUCCESS); 

        default: //è il padre che attende il figlio
            wait(NULL);
            sleep(2);

    }
      

//deallochiamo la memoria condivisa
shmctl(dataId, IPC_RMID,0); 
exit(EXIT_SUCCESS);
}