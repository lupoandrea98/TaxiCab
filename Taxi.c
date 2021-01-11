#include "TaxiFunctions.h"

void handler(int sig){
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
    long TaxiPiuStrada[2] = {0,0}; //TaxiPiuStrada[0] il max di celle attraversate  TaxiPiuStrada[1] il relativo pid del taxi che ha attraversato + celle
    long tripPiuLungo[2] = {0,0}; //TaxiPiuStrada[0] il tempo del viaggio più lungo  TaxiPiuStrada[1] il relativo pid del taxi 
    long richPiuRaccolte[2] = {0,0}; //TaxiPiuStrada[0] il numero di richieste più raccolte  TaxiPiuStrada[1] il relativo pid del taxi 
    //prima della fine di ogni taxi vengono fatti i confronti e aggiornati questi buffer, che conterranno appunto "i record" dei taxi
   
    int valid;      //Flag che controlla se la richiesta è stata soddisfatta o meno (1 = soddisfatta | 0 = insoddisfatta)
    //da sistemare
    SO_TIMEOUT = rand() % 10000;
    int i;

    
    //prelevo ID memoria condivisa
    if((dataid = shmget(SHMKEY, (sizeof(struct cella)*SO_HEIGHT*SO_WIDTH + sizeof(int)*3+ sizeof(long)*2), IPC_CREAT | 0666))==-1){ //creazione shm
        EXIT_ON_ERROR
    }

    //mi attacco al segmento di memoria condivisa
    if((ptMemCond = (struct data*)shmat(dataid, NULL, 0))==(void*)-1){ 
        EXIT_ON_ERROR
    }
    //sleep di prova
    sleep(3);
    //controllo che le sorgenti o i taxi non siano 0
    if((ptMemCond->numTaxi == 0) || (ptMemCond->numSourc == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n"); 
        exit(EXIT_FAILURE);
    } 
    
    //si attacca alla coda di messaggi
    if((codaid = msgget(MSGKEY, 0 ))==-1){ //mi aggancio alla coda (0 xk non mi interessano i flag)
        EXIT_ON_ERROR   
    } 

    if(signal(SIGTERM, handler) == SIG_ERR){
        EXIT_ON_ERROR
    }

    //creo il semaforo
    if((semid = semget(SMFKEY, 1, IPC_CREAT | 0666)) == -1){ //ipc_private crea e mette una chiave a caso, 1 è solo per un set di semafori
        EXIT_ON_ERROR
    }
    // inizializzazione del semaforo
    if(initSemAvailable(semid, 0) <0){
        EXIT_ON_ERROR
    }
  
    
    //printf("Il tempo massimo consentito per soddisfare una richiesta in questa simulazione è: %ld\n", SO_TIMEOUT);


    ptMemCond->TaxiPid = (long) getpid();

        for(i=0;i<3;i++){
            
            switch(fork()){ //solo 1 volta xk 1 taxi, SE NO SO_TAXI VOLTE

                case -1: EXIT_ON_ERROR

                case 0: //è il figlio TAXI
                    infoTaxi.pid = (long)getpid();
                    infoTaxi.percorso = 0;
                    infoTaxi.viaggioPiuLungo = 0;
                    infoTaxi.richiesteRacc = 0;
                    infoTaxi.tempoImpiegato = 0;
                    
                    printf("Sono il taxi! Il mio pid è:");
                    printf(BOLDRED " %ld\n" RESET, infoTaxi.pid);

                    //ricevo la richiesta
                    if(msgrcv(codaid, &coda, sizeof(struct queue), 0,0)==-1){                      
                        printf("Il messaggio ricevuto non esiste, errore!");
                        EXIT_ON_ERROR //il messaggio ricevuto non esiste, errore 
                    } 

                    cercaTaxi(&infoTaxi, ptMemCond->mappa); //cerco una casella con un taxi dentro
                    
                    printf("partenza taxi [%d][%d]\n", infoTaxi.coordTaxi[0], infoTaxi.coordTaxi[1]);
                    printf("sorgente [%d][%d]\n", coda.partenza[0], coda.partenza[1]);
                    printf("e ho ricevuto una richiesta da %ld. La destinazione è [%d][%d]\n", coda.mtype ,coda.arrivo[0], coda.arrivo[1]);
                    infoTaxi.richiesteRacc++;
                    
                    //*************INIZIO SEZIONE CRITICA**************************
                    if(reserveSem(semid, 0)==-1){ //PRENOTO
                        EXIT_ON_ERROR
                    }
                    //controllo se il taxi è già sulle sources
                    if((infoTaxi.coordTaxi[0] == coda.partenza[0]) && (infoTaxi.coordTaxi[1] == coda.partenza[1])){     //Se le coordinate sono diverse, faccio raggiungere la sorgente dal taxi
                        
                        printf("Il taxi [%ld] è già sulla sorgente.\n", (long)getpid());   

                    }else{
                        
                        printf("Il taxi [%ld] non è sulla sorgente e la raggiunge.\n", (long)getpid()); 
                        if(valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.partenza[0], coda.partenza[1], &infoTaxi.tempoImpiegato) == 1){                            
                            printf("Ora il taxi [%ld] è sulla sorgente.\n", (long)getpid()); 
                        }
                        else{                            
                            rispondi(&coda, codaid, valid); //Nel caso in cui valid è zero, informo che la richiesta non è stata soddisfatta
                        }  
                    }

                    if((valid = TaxiMover(ptMemCond->mappa, &infoTaxi, coda.arrivo[0], coda.arrivo[1], &infoTaxi.tempoImpiegato)) == 1){
                        printf("Il taxi [%ld] è arrivato a destinazione.\n", (long)getpid());
                        rispondi(&coda, codaid, valid); //in base al successo/aborto/ecc, da modificare
                    }
                    else{
                        rispondi(&coda, codaid, valid);
                    }

                    printf("Celle percorse dal taxi: %d & Richieste raccolte: %d\n", infoTaxi.percorso, infoTaxi.richiesteRacc);

                    if(releaseSem(semid,0)==-1){ //RILASCIO
                        EXIT_ON_ERROR
                    }
                    //*************FINE SEZIONE CRITICA**************************
                    //DEVO FARE I CONFRONTI E AGGIORNARE GLI ARRAY DEI RECORD!
                    
                    exit(EXIT_SUCCESS);
                   
                /*default: //è il padre                                                           TOGLIEREMO LA DEFAULT PER FAR FORKARE IN PARALLELO TUTTI I FIGLI E LE WAIT LE METTIAMO FUORI 
                    wait(NULL); //attende il figlio
                    sleep(3); //lascia al figlio il tempo di rispondere.
                */
            } //fine switch taxi
        }//fine for

    for(i=0;i<3;i++){
        printf("il taxi[%d] ha terminato\n", wait(NULL));
    }
    


exit(EXIT_SUCCESS);
}