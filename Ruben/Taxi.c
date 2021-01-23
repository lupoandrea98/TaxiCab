#include "TaxiFunctions.h"
//PROCESSO MASTER CHE PRELEVA LA MAPPA DA UNA MEMORIA CONDIVISA, GENERA I TAXI E LI AGGIUNGE ALLA MAPPA IN PIU FORKA SO_TAXI VOLTE PER FARLI DIVENTARE PROCESSI
//INFINE STAMPERÀ I RISULTATI DELLA SIMULAZIONE

void handlerTaxi(int sig){
    if(sig == SIGTERM){
        printf("Simulazione terminata!\n");
        calcolaMax(ptMemCond, &infoTaxi);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]){
   
   
    //da sistemare
    
    int i;

    //prelevo l'ID della memoria condivisa
    if((dataid = shmget(SHMKEY, sizeof(struct data), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }
  
    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }

    //prelevo id semaforo
    if((semid = semget(SMFKEY, 1, 0666)) == -1){ 
        EXIT_ON_ERROR
    }

    //mi aggancio alla coda (0 xk non mi interessano i flag)
    if((codaid = msgget(MSGKEY, 0 ))==-1){ 
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
    
    for(i=0;i<3;i++){
    
        switch(fork()){ //solo 1 volta xk 1 taxi, SE NO SO_TAXI VOLTE

            case -1: EXIT_ON_ERROR

            case 0: //è il figlio TAXI

                execv("TaxiSon", argv);
                //Al momento della exec, tutti i dati relativi al processo precedente vengono SOSTITUITI
                //perciò il codice dopo la exec non verrà mai eseguito.
                //Lascio questa exit failure, perchè nel caso la exec fallisce, allora verrà eseguita questa exit
                exit(EXIT_FAILURE);          
                   
            } //fine switch taxi
    }//fine for

    for(int i=0; i<5; ++i){
        printf("Attendo la terminazione del taxi %d...\n", wait(NULL)); //attendo il figlio
    } 

printf("tutto finito!\n");
exit(EXIT_SUCCESS);
}