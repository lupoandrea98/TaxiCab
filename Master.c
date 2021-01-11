//Programma master che stampa tutte le informazioni in tempo reale della simulazione in corso.
#include "TaxiFunctions.h"

//IL MASTER OLTRE A FARE L'OUTPUT DI TUTTO, DEALLOCHERÀ TUTTO
int main(){
  
    //prelevo l'ID della memoria condivisa
    if((dataid = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*3+ sizeof(long)*2), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

    //si attacca alla coda di messaggi
    if((codaid = msgget(MSGKEY, 0 ))==-1){ //mi aggancio alla coda (0 xk non mi interessano i flag)
        EXIT_ON_ERROR   
    }

    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }

    //controllo che le sorgenti o i taxi non siano 0
    if((ptMemCond->numTaxi == 0) || (ptMemCond->numSourc == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n"); 
        exit(EXIT_FAILURE);
    } 


    //creo il semaforo
    if((semid = semget(SMFKEY, 1, 0666)) == -1){ //ipc_private crea e mette una chiave a caso, 1 è solo per un set di semafori
        EXIT_ON_ERROR
    }
    // inizializzazione del semaforo
    if(initSemAvailable(semid, 0) <0){
        EXIT_ON_ERROR
    }

    while(ptMemCond->durataSimu>0){
        sleep(1);
        printf("Stampa SO_DURATION %d\n", ptMemCond->durataSimu);    
        printMap(ptMemCond->mappa);
        ptMemCond->durataSimu--;
            
    }
    //AMMAZZO GLI ALTRI DUE
    if(kill(ptMemCond->SourcePid, SIGTERM) == -1){
        //EXIT_ON_ERROR
        //Togliamola temporaneamente altrimenti se Source o Taxi termianno il master non dealloca semafori e memoria!
    }
    if(kill(ptMemCond->TaxiPid, SIGTERM) == -1){
        //EXIT_ON_ERROR
    }

    printf("[process with PID %ld] about to remove all IPC\n", (long)getpid());
    // dealloco semaforo
    if(semctl(semid, 0, IPC_RMID, arg)==-1){
        EXIT_ON_ERROR
    }
    //dealloco la coda di messaggi
    if(msgctl(codaid, IPC_RMID, 0) == -1){
        EXIT_ON_ERROR
    }
    //dealloco la memoria condivisa
    if(shmctl(dataid, IPC_RMID,0) == -1){
        EXIT_ON_ERROR
    }   


}