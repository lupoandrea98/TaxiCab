#include "TaxiFunctions.h"
//PROGRAMMA DI SIMULAZIONE CHE GENERA E INIZIALIZZA LA MAPPA (MESSA IN MEMORIA CONDIVISA), FORKA SO_SOURCES E GESTISCE LE RICHIESTE DI TAXI


void handler(int sig){

    if((sig == SIGALRM) || (sig == SIGINT)){

        infoSource.pid = (long) getpid();
        cercaSource(&infoSource, ptMemCond->mappa); //Cerco una casella con la sorgente
        coda.partenza[0] = infoSource.coordSource[0];
        coda.partenza[1] = infoSource.coordSource[1];
        coda.mtype = (long) getpid();
        getCellaArrivo(ptMemCond->mappa, &coda); //metodo che mi dà random delle coordinate di arrivo e me le scrive nella coda di messaggi
        printf("Richiesta! Destinazione: [%d][%d]\n", coda.arrivo[0], coda.arrivo[1]);
        //invio alla coda
        if(msgsnd(codaid, &coda, sizeof(struct queue), IPC_NOWAIT)== -1){
            EXIT_ON_ERROR
        }
        printf("Messaggio inviato correttamente.\n");

    }else if(sig == SIGTERM){
        printf("Simulazione terminata!\n");
        exit(EXIT_SUCCESS);
    }

}

int main(){

    int i; 
    //tutte le altre definizioni (visto che sono pure in comune tra i 3 programmi), sono nell'header!

    //prelevo ID memoria condivisa
    if((dataid = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*3+ sizeof(long)*2), 0666))==-1){ 
        EXIT_ON_ERROR
    }
  
    //attacco alla memoria condivisa 
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ //attacco segmento
        EXIT_ON_ERROR
    }

    //creo la coda
    if((codaid = msgget(MSGKEY, 0666 | IPC_CREAT))== -1){
       EXIT_ON_ERROR 
    }

    SO_DURATION = 20;
    ptMemCond->durataSimu = SO_DURATION;

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

    ptMemCond->SourcePid = (long) getpid();

    if(signal(SIGTERM, handler) == SIG_ERR){
        EXIT_ON_ERROR
    }
    if(signal(SIGALRM, handler) == SIG_ERR){
        EXIT_ON_ERROR
    }
    if(signal(SIGINT, handler) == SIG_ERR){
        EXIT_ON_ERROR
    }    
    

    //forko una volta xk 1 SOURCE di prova, ma dovrò forkare SO_SOURCE volte!
    switch(fork()){

        case -1:
            EXIT_ON_ERROR

        case 0: //è il figlio (RICHIESTA TAXI)   
       
    
            while(ptMemCond->durataSimu>0){
                sleep(4);
                kill(getpid(), SIGALRM);
                
                /*sleep(2);
                //riceve risposta dal taxi
                if(msgrcv(codaId, &coda, sizeof(coda), getpid(), 0) == -1){ 
                    EXIT_ON_ERROR
                }
                printf("Risposta ricevuta! Il figlio del server mi ha scritto '%s'\n", coda.msg);   
                */
            }     
               
             //6.Ricevuta la risposta termina.
            exit(EXIT_SUCCESS); 

        default: //è il padre che attende il figlio
            wait(NULL);
            sleep(2);

    }
//A cosa serve questa sleep?
//sleep(5);

exit(EXIT_SUCCESS);
}