#include "TaxiFunctions.h"
//PROCESSO MASTER CHE PRELEVA LA MAPPA DA UNA MEMORIA CONDIVISA, GENERA I TAXI E LI AGGIUNGE ALLA MAPPA IN PIU FORKA SO_TAXI VOLTE PER FARLI DIVENTARE PROCESSI
//INFINE STAMPERÀ I RISULTATI DELLA SIMULAZIONE

void handlerTaxi(int sig){
    if(sig == SIGTERM){

        //==========SEZIONE CRITICA==============
        if(reserveSem(semid_taxi, 0) == -1){     //Decremento semaforo.
            EXIT_ON_ERROR
        }  
        calcolaMax(ptMemCond, &infoTaxi);
        if(releaseSem(semid_taxi, 0) == -1){     //Incremento semaforo.
            EXIT_ON_ERROR
        }  
        //======================================
        
        printf("Simulazione terminata!\n");
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]){
   
    int i, numTaxi;

    //prelevo l'ID della memoria condivisa
    if((dataid = shmget(SHMKEY, sizeof(struct data), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }
  
    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }

    //Creo un altro set di semafori per i taxi
    if((semid_taxi = semget(SMFKEY_1, 2, IPC_CREAT | 0666)) == -1){ 
        EXIT_ON_ERROR
    }
    //Inizializzo il semaforo dei taxi
    if(initSemAvailable(semid_taxi, 0, 1) == -1){
        EXIT_ON_ERROR
    }
    if(initSemAvailable(semid_taxi, 1, 1) == -1){
        EXIT_ON_ERROR
    }
    
    //gestione per il sigterm
    if(signal(SIGTERM, handlerTaxi) == SIG_ERR){
        EXIT_ON_ERROR
    }   

    ptMemCond->tripSuccess = 0; 
    ptMemCond->tripAborted = 0; 
    ptMemCond->tripNotExec = 0; 
    ptMemCond->TaxiPid = (long) getpgid(getpid()); //per avere arichviato il pid del padre generatore di Taxi
    ptMemCond->TaxiPiuStrada[0] = 0; 
    ptMemCond->tripPiuLungo[0] = 0;
    ptMemCond->richPiuRaccolte[0] = 0;
    ptMemCond->TaxiPiuStrada[1] = 0; 
    ptMemCond->tripPiuLungo[1] = 0;
    ptMemCond->richPiuRaccolte[1] = 0;
    numTaxi = 10;
    printf("I taxi sono %d\n", numTaxi);
    for(i=0;i<numTaxi;i++){
    
        switch(fork()){ //solo 1 volta xk 1 taxi, SE NO SO_TAXI VOLTE

            case -1: EXIT_ON_ERROR

            case 0: //è il figlio TAXI

                execv("TaxiSon", argv); //Se fallisce la exec allora viene fatta una exit failure.
                exit(EXIT_FAILURE);          
                   
            } //fine switch taxi
    }//fine for

    for(i=0; i<numTaxi; ++i){
        printf("Attendo la terminazione del taxi %d...\n", wait(NULL)); //attendo il figlio
    } 

printf("tutto finito!\n");
exit(EXIT_SUCCESS);
}