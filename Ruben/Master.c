//Programma master che stampa tutte le informazioni in tempo reale della simulazione in corso.
#include "TaxiFunctions.h"


void SemaphoreInizialization(int semid){

    if(initSemAvailable(semid, 0) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 1) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 2) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 3) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 4) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 5) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 6) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 7) < 0){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid, 8) < 0){
        EXIT_ON_ERROR
    }

}

void deallocation(int codaid, int dataid, int semid){
    //dealloca semaforo
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
    if(sig == SIGTERM){
        killpg(ptMemCond->SourcePid, SIGTERM);

        killpg(ptMemCond->TaxiPid, SIGTERM);

        deallocation(codaid, dataid, semid);
        
        exit(EXIT_SUCCESS);
    }
}

//IL MASTER OLTRE A FARE L'OUTPUT DI TUTTO, DEALLOCHERÀ TUTTO
int main(){

    /*  
        ======================================================================================================================================================================
        Il processo Master si occupa di allocare ed inizializzare tutti gli strumenti necessari, per poi fermarsi un attimo e permettere l'avvio dei processi Source e Taxi!
        ======================================================================================================================================================================

    */
    signal(SIGTERM, handlerMaster);
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
    if((semid = semget(SMFKEY, 9, IPC_CREAT | 0666)) == -1){ //ipc_private crea e mette una chiave a caso, 1 è solo per un set di semafori
        EXIT_ON_ERROR
    }
    
    SemaphoreInizialization(semid);

    mapGenerator(ptMemCond->mappa);

    HolesGenerator(ptMemCond->mappa);

    SourcesGenerator(ptMemCond->mappa);

    //TaxiGenerator(ptMemCond->mappa);

    printMap(ptMemCond->mappa);

    SO_DURATION = 30;

    ptMemCond->durataSimu = SO_DURATION;
    ptMemCond->SO_TAXI = 0;
    ptMemCond->numSourc = SO_SOURCES;

    printf("Durata prevista per la simulazione: %d\n", SO_DURATION);
    printf("i buchi sono: %d\n", SO_HOLES);
    printf("le sorgenti sono: %d\n", SO_SOURCES);
    //Aspetto per poter avviare correttamente i processi Source e Taxi
    sleep(5);
    
    //controllo che le sorgenti o i taxi non siano 0
    if((ptMemCond->numSourc == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n"); 
        //Anzichè fare una exit, impostiamo un handler che prima manda la kill agli altri due e poi fa la exit(EXIT_SUCCESS)
        if(killpg(ptMemCond->SourcePid, SIGTERM) == -1){
            printf("Problema kill Source\n");
            deallocation(codaid, dataid, semid);
        }
        if(killpg(ptMemCond->TaxiPid, SIGTERM) == -1){
            printf("Problema kill Taxi\n");
            deallocation(codaid, dataid, semid);
        }
        exit(EXIT_FAILURE);
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
    killpg(ptMemCond->SourcePid, SIGTERM);

    killpg(ptMemCond->TaxiPid, SIGTERM);

    printf("Stampo resoconto simulazione: \n");

    printf("Il Taxi che ha percorso più strada è: %ld, ed ha percorso %ld celle\n",  ptMemCond->TaxiPiuStrada[1], ptMemCond->TaxiPiuStrada[0]);
    printf("Il Taxi che ha impiegato più tempo per portare a termine la richiesta è: %ld, ed ci ha impiegato %ld nsec\n", ptMemCond->tripPiuLungo[1], ptMemCond->tripPiuLungo[0]);
    printf("Il Taxi che ha raccolto più richieste è: %ld e ne ha raccolte %ld\n", ptMemCond->richPiuRaccolte[1], ptMemCond->richPiuRaccolte[0]);
    printf("Numero viaggi tot: %d\n", (ptMemCond->tripSuccess + ptMemCond->tripAborted + ptMemCond->tripNotExec));
    printf("Numero viaggi abortiti: %d\n", ptMemCond->tripAborted);
    printf("Numero viaggi eseguiti: %d\n", ptMemCond->tripSuccess);
    printf("Numero viaggi non eseguiti: %d\n", ptMemCond->tripNotExec);
    deallocation(codaid, dataid, semid);

    exit(EXIT_SUCCESS);
    

   
}