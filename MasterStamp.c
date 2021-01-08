//Programma master che stampa tutte le informazioni in tempo reale della simulazione in corso.
#include "TaxiFunctions.h"

int main(){
    //Devo attaccarmi alla memoria condivisa per poter leggere in tempo reale tutti i dati.
    int msgid,dataId, contatore;
    struct queue coda;
    struct data* ptMemCond;
    
    //prelevo l'ID della memoria condivisa
    if((dataId = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*2), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

   //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataId, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }
    contatore = 0;
    for(;;){
        sleep(1);
        printf("Stampa %d\n", contatore++);
        printMap(ptMemCond->mappa);
    }

}