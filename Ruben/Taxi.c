#include "TaxiFunctions.h"
//PROCESSO MASTER CHE PRELEVA LA MAPPA DA UNA MEMORIA CONDIVISA, GENERA I TAXI E LI AGGIUNGE ALLA MAPPA IN PIU FORKA SO_TAXI VOLTE PER FARLI DIVENTARE PROCESSI
//INFINE STAMPERÀ I RISULTATI DELLA SIMULAZIONE

void handlerTaxi(int sig){
    if(sig == SIGTERM){
        printf("Simulazione terminata!\n");
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char *argv[]){

 
    //viaggi. Numeri di: (la somma di queste 3 variabili mi dà il numero di viaggi totali di tutti i taxi, mi servirà per la stampa finale)
    int tripSuccess = 0; //richieste completate con successo
    int tripAborted = 0; //richieste abortite (SO_TIMEOUT taxi, il tempo è scaduto) "tempo di inattività del taxi dopo il quale il taxi muore" (muore xk è scaduto il tempo nella cella?)
    int tripNotExec = 0; //richieste non intraprese
    //buffer che ci serviranno per l'output:

    //DA METTERE POI IN MEMORIA CONDIVISA PER IL MASTER!!
    long TaxiPiuStrada[2] = {0,0}; //TaxiPiuStrada[0] il max di celle attraversate  TaxiPiuStrada[1] il relativo pid del taxi che ha attraversato + celle
    long tripPiuLungo[2] = {0,0}; //TaxiPiuStrada[0] il tempo del viaggio più lungo  TaxiPiuStrada[1] il relativo pid del taxi 
    long richPiuRaccolte[2] = {0,0}; //TaxiPiuStrada[0] il numero di richieste più raccolte  TaxiPiuStrada[1] il relativo pid del taxi 
    //prima della fine di ogni taxi vengono fatti i confronti e aggiornati questi buffer, che conterranno appunto "i record" dei taxi
   
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
   
    ptMemCond->TaxiPid = (long) getpgid(getpid()); //per avere arichviato il pid del padre generatore di Taxi
    
    
       // for(i=0;i<3;i++){
            
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
                    }
                     /* if(releaseSem(semid,0)==-1){ //RILASCIO
                            EXIT_ON_ERROR
                    }*/
                        //*************FINE SEZIONE CRITICA************************** 

                        //DEVO FARE I CONFRONTI E AGGIORNARE GLI ARRAY DEI RECORD!
                        //printf("vita del Taxi %ld esaurita!\n", (long)getpid());
                        //-FUNZIONE CHE GENERA UN NUOVO TAXI NELLA MAPPA (oneTaxiGenerator)
                    //} //while infinto interno al taxi
                    printf("Il taxi %ld Termina! Totale celle percorse: %d & Richieste raccolte: %d\n", infoTaxi.pid, infoTaxi.percorso, infoTaxi.richiesteRacc);
                    exit(EXIT_SUCCESS);          
                

                default:
                    printf("Attendo la terminazione del taxi %d...\n", wait(NULL));
                    

       
            } //fine switch taxi
        //}//fine for

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