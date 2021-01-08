//Programma master che stampa tutte le informazioni in tempo reale della simulazione in corso.
#include "TaxiFunctions.h"

//IL MASTER OLTRE A FARE L'OUTPUT DI TUTTO, DEALLOCHERÀ TUTTO
int main(){
    //Devo attaccarmi alla memoria condivisa per poter leggere in tempo reale tutti i dati.
    int msgid,dataId, semid, contatore;
    struct queue coda;
    struct data* ptMemCond;
    union semun arg; //da mettere di default, per la rimozione del semaforo finale
    
    //prelevo l'ID della memoria condivisa
    if((dataId = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*2), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

   //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataId, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }

//creo il semaforo
  if((semid = semget(SMFKEY, 1, 0666)) == -1){ //ipc_private crea e mette una chiave a caso, 1 è solo per un set di semafori
    EXIT_ON_ERROR
  }
  // inizializzazione del semaforo
  if(initSemAvailable(semid, 0) <0){
    EXIT_ON_ERROR
  }

    contatore = 0;
    for(;;){
        sleep(1);
        printf("Stampa %d\n", contatore++);
            /*if(reserveSem(semid, 0)==-1){ //PRENOTO
                     EXIT_ON_ERROR
                    }*/
        printMap(ptMemCond->mappa);
         /*if(releaseSem(semid,0)==-1){ //RILASCIO
                        EXIT_ON_ERROR
                    }*/
    }

     // rimozione semaforo
  printf("[process with PID %ld] about to remove semaphore\n", (long)getpid());
  if(semctl(semid, 0, IPC_RMID, arg)==-1){
    EXIT_ON_ERROR
  }
    


}