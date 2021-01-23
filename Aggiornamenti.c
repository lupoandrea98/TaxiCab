#include "TaxiFunctions.h"
//PROCESSO MASTER CHE PRELEVA LA MAPPA DA UNA MEMORIA CONDIVISA, GENERA I TAXI E LI AGGIUNGE ALLA MAPPA IN PIU FORKA SO_TAXI VOLTE PER FARLI DIVENTARE PROCESSI
//INFINE STAMPERÀ I RISULTATI DELLA SIMULAZIONE

void handlerTaxi(int sig){
    if(sig == SIGTERM){
        printf("Simulazione terminata!\n");
        exit(EXIT_SUCCESS);
    }
}

void calcolaMax(struct data* pt , struct* taxi taxi){
  
  if(pt->TaxiPiuStrada[0]<taxi->percorso){
      pt->TaxiPiuStrada[0] = taxi->percorso;
      pt->TaxiPiuStrada[1] = taxi->pid;
  }

  if(pt->tripPiuLungo[0]<taxi->tempoImpiegato){
      pt->tripPiuLungo[0] = taxi->tempoImpiegato;
      pt->tripPiuLungo[1] = taxi->pid;
  }

  if(pt->richPiuRaccolte[0]<taxi->richiesteRacc){
      pt->richPiuRaccolte[0] = taxi->richiesteRacc;
      pt->richPiuRaccolte[0] = taxi->pid;
  }
       
}



int main(int argc, char *argv[]){

 
    
   
    int valid;      //Flag che controlla se la richiesta è stata soddisfatta o meno (1 = soddisfatta | 0 = insoddisfatta)
    //da sistemare
    SO_TIMEOUT = rand() % 10000;
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
                
                    printf("Sono il taxi! Il mio pid è: ");
                    printf(BOLDRED "%ld\n" RESET, (long)getpid());
                    cercaTaxi(&infoTaxi, ptMemCond->mappa); //cerco una casella con un taxi dentro
                    printf("Le mie cordinate: {%d}{%d}\n", infoTaxi.coordTaxi[0],infoTaxi.coordTaxi[1]);
                    //while(ptMemCond->durataSimu>0){    
                    printf("%ld in attesa di richieste...\n", (long)getpid());
                    //ricevo la richiesta
                    if(msgrcv(codaid, &coda, sizeof(struct queue), 0,0)==-1){                      
                        printf("Errore msgrcv!\n");
                        EXIT_ON_ERROR 
                    }
                        
                    printf("Sono %ld, messaggio ricevuto! \n", (long)getpid());
                        //verifico che non sia una richiesta di terminazione
                    if(coda.mtype==1){ 
                        printf("%d ha ricevuto una richiesta di terminazione!\n", getpid());
                        exit(EXIT_SUCCESS);
                    }
                    /*if(reserveSem(semid, 0)==-1)
                        EXIT_ON_ERROR       
                       
                    //printf("Ho ricevuto una richiesta da %ld. La destinazione è [%d][%d]\n", coda.mtype ,coda.arrivo[0], coda.arrivo[1]);          
                       
                        
                    if(releaseSem(semid,0)==-1) 
                        EXIT_ON_ERROR*/
                        
                    //inizializzo i dati del figlio taxi
                    infoTaxi.pid = (long)getpid();
                    infoTaxi.percorso = 0;
                    infoTaxi.viaggioPiuLungo = 0;
                    infoTaxi.richiesteRacc = 0;
                    infoTaxi.tempoImpiegato = 0;
                        
                    printf("Partenza taxi [%d][%d]\n", infoTaxi.coordTaxi[0], infoTaxi.coordTaxi[1]);
                    printf("Sorgente [%d][%d]\n", coda.partenza[0], coda.partenza[1]);
                    printf("Ho ricevuto una richiesta da %ld. La destinazione è [%d][%d]\n", coda.mtype ,coda.arrivo[0], coda.arrivo[1]);
                    infoTaxi.richiesteRacc++;
                    
                    //controllo se il taxi è già sulle source
                    if((infoTaxi.coordTaxi[0] == coda.partenza[0]) && (infoTaxi.coordTaxi[1] == coda.partenza[1])){     //Se le coordinate sono diverse, faccio raggiungere la sorgente dal taxi
                            
                        printf("Il taxi [%ld] è già sulla sorgente.\n", (long)getpid());   

                    }else{
                            
                        printf("Il taxi [%ld] non è sulla sorgente e la raggiunge.\n", (long)getpid()); 
                        //*************INIZIO SEZIONE CRITICA**************************
                                  /*  if(reserveSem(semid, 0)==-1){ //PRENOTO
                                        EXIT_ON_ERROR
                                    }*/
                        if(valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.partenza[0], coda.partenza[1], &infoTaxi.tempoImpiegato) == 1){                            
                            printf("Ora il taxi [%ld] è sulla sorgente.\n", (long)getpid()); 
                        }else{  //RICHIESTA FALLITA
                            //Aggiornare mappa (il taxi scompare dalla ultima cella)
                            //SO_TAXI--
                            calcoloMax(&ptMemCond, &infoTaxi);
                            //funzione che forka (al suo interno oneTaxi generator)
                            ptMemCond->tripAborted++;
                            //exit(EXIT_SUCCESS);
                        }

                        /*  if(releaseSem(semid,0)==-1){ //RILASCIO
                                EXIT_ON_ERROR
                            } 
                        */
                            //*************FINE SEZIONE CRITICA**************************               
                    }
                        //*************INIZIO SEZIONE CRITICA**************************
                        /*  if(reserveSem(semid, 0)==-1){ //PRENOTO
                                EXIT_ON_ERROR
                            }
                        */

                    if((valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.arrivo[0], coda.arrivo[1], &infoTaxi.tempoImpiegato)) == 1){
                        printf("Il taxi [%ld] è arrivato a destinazione.\n", (long)getpid());
                    }else{  //RICHIESTA FALLITA
                        //Aggiornare mappa (il taxi scompare dalla ultima cella)
                        //SO_TAXI--
                        //
                       calcoloMax(&ptMemCond, &infoTaxi);
                        //funzione che forka (al suo interno oneTaxi generator)
                        ptMemCond->tripAborted++;
                        //exit(EXIT_SUCCESS);
                    }
                     /* if(releaseSem(semid,0)==-1){ //RILASCIO
                            EXIT_ON_ERROR
                    }*/
                        //*************FINE SEZIONE CRITICA************************** 

                        //DEVO FARE I CONFRONTI E AGGIORNARE GLI ARRAY DEI RECORD!
                        //printf("vita del Taxi %ld esaurita!\n", (long)getpid());
                        //-FUNZIONE CHE GENERA UN NUOVO TAXI NELLA MAPPA (oneTaxiGenerator)
                    //} //while infinto interno al taxi
                    calcoloMax(&ptMemCond, &infoTaxi);
                    ptMemCond->tripSuccess++;
                    printf("Il taxi %ld Termina! Totale celle percorse: %d & Richieste raccolte: %d\n", infoTaxi.pid, infoTaxi.percorso, infoTaxi.richiesteRacc);
                    exit(EXIT_SUCCESS);          
                

                default:
                    printf("Attendo la terminazione del taxi %d...\n", wait(NULL));
                    

       
            } //fine switch taxi
        }//fine for

    //for(int i=0; i<3; ++i){
    // printf("Attendo la terminazione del taxi %d...\n", wait(NULL)); //attendo il figlio
    // } 
    
    // rimozione semaforo (GIÀ PRESENTE NEL MASTER!)
    /*if(semctl(semid, 0, IPC_RMID, arg)==-1){
        EXIT_ON_ERROR
    }
    */

printf("tutto finito!\n");
exit(EXIT_SUCCESS);
}