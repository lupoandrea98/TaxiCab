#include "TaxiFunctions.h"

int valid;                       //Flag che controlla se la richiesta è stata soddisfatta o meno (1 = soddisfatta | 0 = insoddisfatta)

void handlerTaxi2(int sig){
    if(sig == SIGTERM){
        if(valid != 1){
            ptMemCond->tripAborted++;
        }
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


int main(){

    
    SO_TIMEOUT = rand() % 10000;
    //gestione per il sigterm
    if(signal(SIGTERM, handlerTaxi2) == SIG_ERR){
        EXIT_ON_ERROR
    }   
    //prelevo l'ID della memoria condivisa
    if((dataid = shmget(SHMKEY, sizeof(struct data), 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }
  
    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }

    //mi aggancio alla coda (0 xk non mi interessano i flag)
    if((codaid = msgget(MSGKEY, 0 ))==-1){ 
        EXIT_ON_ERROR     
    }
    //prelevo id semaforo
    if((semid = semget(SMFKEY, 1, 0666)) == -1){ 
        EXIT_ON_ERROR
    }
    //prelevo id semaforo
    if((semid_taxi = semget(SMFKEY_1, 1, 0666)) == -1){ 
        EXIT_ON_ERROR
    }

    printf("Sono il taxi! Il mio pid è: ");
    printf(BOLDRED "%ld\n" RESET, (long)getpid());
    //reset();

    //Andando a modificare la memoria condivisa con il taxi generator, lo mettiamo all'interno di una sezione critica
    //================SEZIONE CRITICA=========================
    if(reserveSem(semid, 0) == -1){     //Decremento semaforo.
        EXIT_ON_ERROR
    }  

    TaxiGenerator(ptMemCond->mappa, &infoTaxi);

    if(releaseSem(semid, 0) == -1){     //Incremento semaforo.
        EXIT_ON_ERROR
    } 
    //======================================================== 
    printf("Le mie cordinate: {%d}{%d}\n", infoTaxi.coordTaxi[0],infoTaxi.coordTaxi[1]);

    
    //WHILE CON CONDIZIONE VITA TAXI
    while(infoTaxi.tempoImpiegato<SO_TIMEOUT){

        printf("%ld in attesa di richieste...\n", (long)getpid());
        //===============SEZIONE CRITICA==================
        if(reserveSem(semid_taxi, 1) == -1){     //Decremento semaforo.
            EXIT_ON_ERROR
        }  
        //ricevo la richiesta
        if(msgrcv(codaid, &coda, sizeof(struct queue), 0,0)==-1){                      
            EXIT_ON_ERROR 
        }    
        printf("Sono %ld, messaggio ricevuto! \n", (long)getpid());
        if(releaseSem(semid_taxi, 1) == -1){     //Incremento semaforo.
            EXIT_ON_ERROR
        } 
        //=================================================
        //verifico che non sia una richiesta di terminazione
        if(coda.mtype==1){ 
            printf("%d ha ricevuto una richiesta di terminazione!\n", getpid());

            //==========SEZIONE CRITICA==============
            if(reserveSem(semid_taxi, 0) == -1){     //Decremento semaforo.
                EXIT_ON_ERROR
            }  
            calcolaMax(ptMemCond, &infoTaxi);
            if(releaseSem(semid_taxi, 0) == -1){     //Incremento semaforo.
                EXIT_ON_ERROR
            }  
            //======================================

            exit(EXIT_SUCCESS);
        }
        ptMemCond->tripNotExec--;
    
            
        printf("Partenza taxi [%d][%d]\n", infoTaxi.coordTaxi[0], infoTaxi.coordTaxi[1]);
        printf("Sorgente [%d][%d]\n", coda.partenza[0], coda.partenza[1]);
        printf("Ho ricevuto una richiesta da %ld. La destinazione è [%d][%d]\n", coda.mtype ,coda.arrivo[0], coda.arrivo[1]);
        infoTaxi.richiesteRacc++;
        
        //controllo se il taxi è già sulle source
        if((infoTaxi.coordTaxi[0] == coda.partenza[0]) && (infoTaxi.coordTaxi[1] == coda.partenza[1])){     //Se le coordinate sono diverse, faccio raggiungere la sorgente dal taxi
                
            printf("Il taxi [%ld] è già sulla sorgente.\n", (long)getpid());   

        }else{
                
            printf("Il taxi [%ld] non è sulla sorgente e la raggiunge.\n", (long)getpid()); 

            if((valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.partenza[0], coda.partenza[1], &infoTaxi.tempoImpiegato)) == 1){                            
                printf("Ora il taxi [%ld] è sulla sorgente.\n", (long)getpid()); 
                valid = 0;
            }else{  //RICHIESTA FALLITA
                ptMemCond->SO_TAXI--;

                //==========SEZIONE CRITICA==============
                if(reserveSem(semid_taxi, 0) == -1){     //Decremento semaforo.
                    EXIT_ON_ERROR
                }  
                calcolaMax(ptMemCond, &infoTaxi);
                if(releaseSem(semid_taxi, 0) == -1){     //Incremento semaforo.
                    EXIT_ON_ERROR
                }  
                //======================================

                ptMemCond->tripAborted++;
            
                switch(fork()){
                    case -1:
                        EXIT_ON_ERROR
                    case 0:
                        execv("TaxiSon", argv);
                    default:
                        wait(NULL);
                        exit(EXIT_SUCCESS);
                }
            }
                
        }

        if((valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.arrivo[0], coda.arrivo[1], &infoTaxi.tempoImpiegato)) == 1){
            printf("Il taxi [%ld] è arrivato a destinazione.\n", (long)getpid());
        }else{  //RICHIESTA FALLITA
            
            ptMemCond->SO_TAXI--;
    
            //==========SEZIONE CRITICA==============
            if(reserveSem(semid_taxi, 0) == -1){     //Decremento semaforo.
                EXIT_ON_ERROR
            }  
            calcolaMax(ptMemCond, &infoTaxi);
            if(releaseSem(semid_taxi, 0) == -1){     //Incremento semaforo.
                EXIT_ON_ERROR
            }  
            //======================================

            ptMemCond->tripAborted++;
    
            switch(fork()){
                case -1:
                    EXIT_ON_ERROR
                case 0:
                    execv("TaxiSon", argv);
                default:
                    wait(NULL);
                    exit(EXIT_SUCCESS);
            }
        }

        calcolaMax(ptMemCond, &infoTaxi);

        ptMemCond->tripSuccess++;
    } //Fine while
    printf("Il taxi %ld Termina! Totale celle percorse: %d & Richieste raccolte: %d\n", infoTaxi.pid, infoTaxi.percorso, infoTaxi.richiesteRacc);
    exit(EXIT_SUCCESS);
}

