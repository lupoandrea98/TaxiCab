//Programma master che stampa tutte le informazioni in tempo reale della simulazione in corso.
#include "TaxiFunctions.h"

void deallocation(int codaid, int dataid, int semid){
    //dealloca semaforo
    printf("[process with PID %ld] about to remove all IPC\n", (long)getpid());
    if(semctl(semid, 0, IPC_RMID, arg)==-1){
        EXIT_ON_ERROR
    }
    //dealloca la coda
    if(msgctl(codaid, IPC_RMID, 0) == -1){ //dealloca
        EXIT_ON_ERROR
    }
                            
    //dealloca memoria condivisa
    if(shmctl(dataid, IPC_RMID,0) == -1){
        EXIT_ON_ERROR
    }

    printf("Tutto deallocato, addio.\n");
}

void handlerMaster(int sig){
    if(sig == SIGINT){
        //AMMAZZO GLI ALTRI DUE
        if(kill(ptMemCond->SourcePid, SIGTERM) == -1){
            EXIT_ON_ERROR
        }
        if(kill(ptMemCond->TaxiPid, SIGTERM) == -1){
            EXIT_ON_ERROR
        }
    }
    deallocation(codaid, dataid, semid);
    exit(EXIT_FAILURE);
}

//IL MASTER OLTRE A FARE L'OUTPUT DI TUTTO, DEALLOCHERÀ TUTTO
int main(){
    
    signal(SIGINT, handlerMaster);
    /*  
        ======================================================================================================================================================================
        Il processo Master si occupa di allocare ed inizializzare tutti gli strumenti necessari, per poi fermarsi un attimo e permettere l'avvio dei processi Source e Taxi!
        ======================================================================================================================================================================

    */
    //creo la coda di messaggi
    if((codaid = msgget(MSGKEY, 0666 | IPC_CREAT))== -1){
       EXIT_ON_ERROR 
    }

    //creo la memoria condivisa
    if((dataid = shmget(SHMKEY, sizeof(struct data),  IPC_CREAT | 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }   

    //creo il semaforo
    if((semid = semget(SMFKEY, 1, IPC_CREAT | 0666)) == -1){ //ipc_private crea e mette una chiave a caso, 1 è solo per un set di semafori
        EXIT_ON_ERROR
    }
    // inizializzazione del semaforo
    if(initSemAvailable(semid, 0) < 0){
        EXIT_ON_ERROR
    }

    mapGenerator(ptMemCond->mappa);

    HolesGenerator(ptMemCond->mappa);

    SourcesGenerator(ptMemCond->mappa);

    TaxiGenerator(ptMemCond->mappa);

    printMap(ptMemCond->mappa);

    SO_DURATION = 10;

    ptMemCond->durataSimu = SO_DURATION;
    ptMemCond->numTaxi = SO_TAXI;
    ptMemCond->numSourc = SO_SOURCES;

    printf("Durata prevista per la simulazione: %d\n", SO_DURATION);
    printf("i buchi sono: %d\n", SO_HOLES);
    printf("le sorgenti sono: %d\n", SO_SOURCES);
    printf("i taxi sono: %d\n", SO_TAXI); //se ne vedi di meno sulla mappa è perché ce ne sono di più in una cella

    //Aspetto per poter avviare correttamente i processi Source e Taxi
    sleep(5);

    //controllo che le sorgenti o i taxi non siano 0
    if((ptMemCond->numTaxi == 0) || (ptMemCond->numSourc == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n"); 
        //Anzichè fare una exit, impostiamo un handler che prima manda la kill agli altri due e poi fa la exit(EXIT_SUCCESS)
        kill(getpid(), SIGINT);
        //exit(EXIT_FAILURE);
    } 

    while(ptMemCond->durataSimu>0){
        sleep(1);
        printf("Stampa SO_DURATION %d\n", ptMemCond->durataSimu);    
        printMap(ptMemCond->mappa);
        ptMemCond->durataSimu--;
         
    }
    /*  
        QUANDO UCCIDIAMO UN PROCESSO PADRE, I FIGLI DIVENTANO ORFANI E VENGONO ASSEGNATI AD UN NUOVO PADRE. 

    */

    //AMMAZZO GLI ALTRI DUE
    if(killpg(ptMemCond->SourcePid, SIGTERM) == -1){
        printf("Problema kill Source\n");
        deallocation(codaid, dataid, semid);
        exit(EXIT_FAILURE);
    }
    if(killpg(ptMemCond->TaxiPid, SIGTERM) == -1){
        printf("Problema kill Taxi\n");
        deallocation(codaid, dataid, semid);
        exit(EXIT_FAILURE);
    }
    /*if(SO_DURATION<=0){
        printf("Tempo simulazione scaduto!\n");
    }
    */

    deallocation(codaid, dataid, semid);
   
}