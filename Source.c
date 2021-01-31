#include "TaxiFunctions.h"

void handlerSource(int sig){

    if((sig == SIGALRM) || (sig == SIGINT) && ptMemCond->durataSimu>0){
      
        
        //==============SEZIONE CRITICA=====================
        if(reserveSem(semid, 0) == -1){     //Decremento semaforo.
            EXIT_ON_ERROR
        } 
        cercaSource(&infoSource, ptMemCond->mappa); //Cerco una casella con la sorgente

        //carico la coda:
        coda.partenza[0] = infoSource.coordSource[0];
        coda.partenza[1] = infoSource.coordSource[1];
        coda.mtype = (long) getpid();
        getCellaArrivo(ptMemCond->mappa, &coda); //metodo che mi dà random delle coordinate di arrivo e me le scrive nella coda di messaggi
        printf("Richiesta! Destinazione: [%d][%d]\n", coda.arrivo[0], coda.arrivo[1]);
        //invio alla coda
        if(msgsnd(codaid, &coda, (sizeof(struct queue)), IPC_NOWAIT)== -1){
            EXIT_ON_ERROR
        }
        ptMemCond->tripNotExec++;
        printf("Messaggio inviato correttamente.\n");
        if(releaseSem(semid, 0) == -1){     //Decremento semaforo.
            EXIT_ON_ERROR
        } 
        //==================================================
    
    }else if(sig == SIGTERM){
        printf("Simulazione terminata!\n");
        exit(EXIT_SUCCESS);
    }

}

int main(){

    int i;
    int durata; 
    //tutte le altre definizioni (visto che sono pure in comune tra i 3 programmi), sono nell'header!
    //(sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*3+ sizeof(long)*2)
    //Ottengo la memoria condivisa
    if((dataid = shmget(SHMKEY, sizeof(struct data) , 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }
  
    //attacco alla memoria condivisa 
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ //attacco segmento
        EXIT_ON_ERROR
    }

    //mi aggancio alla coda (0 xk non mi interessano i flag)
    if((codaid = msgget(MSGKEY, 0 ))==-1){ 
        EXIT_ON_ERROR     
    }

    ptMemCond->SourcePid = (long) getpgid(getpid()); //per avere arichviato il pid del padre SOURCE

    if(signal(SIGTERM, handlerSource) == SIG_ERR){
        EXIT_ON_ERROR
    }
    if(signal(SIGALRM, handlerSource) == SIG_ERR){
        EXIT_ON_ERROR
    }
    if(signal(SIGINT, handlerSource) == SIG_ERR){
        EXIT_ON_ERROR
    }    
    
    //ottengo il semaforo
    if((semid = semget(SMFKEY, 1, 0666)) == -1){ 
        EXIT_ON_ERROR
    }
   
    switch(fork()){

        case -1:

            EXIT_ON_ERROR

        case 0: //è il figlio (RICHIESTA TAXI)   

            printf("Le sorgenti cominciano a mandare richieste...\n");
            while(ptMemCond->durataSimu>0){ //sarà infinito, esce quando il master setta il flag nella memoria condivisa
                sleep(3);
                kill(getpid(), SIGALRM);
            }     
            exit(EXIT_SUCCESS); 

        default:

            wait(NULL); //attendo il figlio

    }

    printf("SO SOURCE HA FINITO\n");
    sleep(5);
    printf("mando una send per dire che ho finito.\n");

    coda.mtype = 1;  
    if(msgsnd(codaid, &coda, (sizeof(struct queue)), IPC_NOWAIT)== -1){
        EXIT_ON_ERROR
    }

exit(EXIT_SUCCESS);

}