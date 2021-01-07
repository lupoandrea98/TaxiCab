//Pseudo codice per memoria condivisa del progetto

#include "TaxiFunction.h"

struct data{
    struct cella mappa[SO_HEIGHT][SO_WIDTH];
};

int main(){
    
    key_t key = 555;
    int shm_id;
    struct data *mappa1;
    struct data *mappa2;
    //Alloco la mia memoria condivisa.
    if(shm_id = shmget(key, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH), IPC_CREAT | 0666) == -1){
        EXIT_ON_ERROR
    }
    mappa1 = (struct data*) shmat(shm_id, NULL, 0);
    printf("Memoria allocata correttamente\n");
    mapGenerator(mappa1->mappa);
    printf("PID[%ld]\n", (long)getpid());
    printf("Stampo la mappa prima di passarla alla memoria condivisa\n");
    printMap(mappa1->mappa);

    //Attacco alla memoria condivisa la mappa creata

    //Forko creando un processo figlio nel tentativo di passargli la mappa tramite memoria condivisa.

    switch(fork()){

        case -1:

            EXIT_ON_ERROR

        case 0: // ================ CODICE DEL FIGLIO =====================

            printf("PID[%ld]\n", (long)getpid());
            printf("Sono il figlio, mi attacco alla memoria condivisa per prelevare la mappa generata da mio padre\n");
            
            mappa2 = (void*) shmat(shm_id, NULL, 0);

            printf("Stampo la mappa prelevata dalla memoria condivisa\n");
            printMap(mappa2->mappa);

            exit(EXIT_SUCCESS);

        default: // ================ CODICE DEL PADRE =====================

            printf("PID[%ld]\n", (long)getpid());

            //Attendo la terminazione del figlio.

            wait(NULL);

            //Dealloco la memoria condivisa.

            if(shmctl(shm_id, IPC_RMID, NULL) == -1){
                EXIT_ON_ERROR
            }


    }


}