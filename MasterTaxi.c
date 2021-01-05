#include "TaxiFunction.h"
//PROCESSO MASTER CHE PRELEVA LA MAPPA DA UNA MEMORIA CONDIVISA, GENERA I TAXI E LI AGGIUNGE ALLA MAPPA IN PIU FORKA SO_TAXI VOLTE PER FARLI DIVENTARE PROCESSI
//INFINE STAMPERÀ I RISULTATI DELLA SIMULAZIONE

int main(int argc, char *argv[]){

    int msgid;
    struct queue coda;
    struct cella mappa[SO_HEIGHT][SO_WIDTH]; //tramite memoria condivisa mi prendo la mappa generata da "Simulator.c"
    //int partenzaTaxi[2]; //mi servirà per quando parte un taxi 
    int coordSource[2]; //coordinate sorgenti
    //viaggi. Numeri di: (la somma di queste 3 variabili mi dà il numero di viaggi totali di tutti i taxi, mi servirà per la stampa finale)
    int tripSuccess = 0; //richieste completate con successo
    int tripAborted = 0; //richieste abortite (SO_TIMEOUT taxi, il tempo è scaduto) "tempo di inattività del taxi dopo il quale il taxi muore" (muore xk è scaduto il tempo nella cella?)
    int tripNotExec = 0; //richieste non intraprese
    //buffer che ci serviranno per l'output:
    long TaxiPiuStrada[2] = {0,0}; //TaxiPiuStrada[0] il max di celle attraversate  TaxiPiuStrada[1] il relativo pid del taxi che ha attraversato + celle
    long tripPiuLungo[2] = {0,0}; //TaxiPiuStrada[0] il tempo del viaggio più lungo  TaxiPiuStrada[1] il relativo pid del taxi 
    long richPiuRaccolte[2] = {0,0}; //TaxiPiuStrada[0] il numero di richieste più raccolte  TaxiPiuStrada[1] il relativo pid del taxi 
    //prima della fine di ogni taxi vengono fatti i confronti e aggiornati questi buffer, che conterranno appunto "i record" dei taxi
    struct taxi infoTaxi;
    SO_TIMEOUT = 10000;
        
    //creo la coda
    if((msgid = msgget(MSGKEY, 0666 | IPC_CREAT))== -1) 
       EXIT_ON_ERROR 

    printf("Coda creata con ID: %d. Mi metto in attesa di richieste da parte di celle SO_SOURCES\n...\n", msgid);

        if(msgrcv(msgid, &coda, sizeof(coda), 0,0)==-1){ 

            printf("Il messaggio ricevuto non esiste, errore!");
            EXIT_ON_ERROR //il messaggio ricevuto non esiste, errore 

        }else{

        //SE I TAXI GENERATI SONO 0 LA SIMULAZIONE DEVE TERMINARE!
        //STESSA COSA PER LE SORGENTI!
        //Termina solo il Simulator, MasterTaxi continua, eseguendo il taxi mover che va in loop
        if(coda.numSource == 0 || coda.numTaxi == 0){
            printf("Non è possibile proseguire la simulazione! Termino.\n");
            exit(EXIT_FAILURE);
            
        }

            switch(fork()){ //solo 1 volta xk 1 taxi, SE NO SO_TAXI VOLTE

                case -1: EXIT_ON_ERROR

                case 0: //è il figlio
                    assignMat(coda.map ,mappa); //ci riassegnamo la mappa, perché poi il messaggio è consumato
                    cercaSource(coordSource, mappa); //Cerco una casella con la sorgente
                    cercaTaxi(&infoTaxi, mappa); //cerco una casella con un taxi dentro
                   
                    infoTaxi.pid = (long)getpid();
                    infoTaxi.percorso = 0;
                    infoTaxi.viaggioPiuLungo = 0;
                    infoTaxi.richiesteRacc = 0;
                    
                    
                    printf("partenza taxi [%d][%d]\n", infoTaxi.coordTaxi[0], infoTaxi.coordTaxi[1]);
                    printf("sorgente [%d][%d]\n", coordSource[0], coordSource[1]);
                    printf("Sono il taxi! Il mio pid è: %ld\n", infoTaxi.pid );
                    printf("e ho ricevuto una richiesta da %ld. La destinzazione è [%d][%d]\n", coda.mtype ,coda.arrivo[0], coda.arrivo[1]);
                    infoTaxi.richiesteRacc++;

                    //controllo se il taxi è giò sulle sources
                    if((infoTaxi.coordTaxi[0] == coordSource[0]) && (infoTaxi.coordTaxi[1] == coordSource[1])){     //Se le coordinate sono diverse, faccio raggiungere la sorgente dal taxi
                        
                        printf("Il taxi [%ld] è già sulla sorgente.\n", (long)getpid());   
                    }else{
                        
                        printf("Il taxi [%ld] non è sulla sorgente e la raggiunge.\n", (long)getpid()); 
                        if(TaxiMover(mappa, &infoTaxi, coordSource[0], coordSource[1])==1){
                            printf("Ora il taxi [%ld] è sulla sorgente.\n", (long)getpid()); 
                        }
                        else{
                            EXIT_ON_ERROR
                        }  
                    }

                    if(TaxiMover(mappa, &infoTaxi, coda.arrivo[0], coda.arrivo[1])==1){
                            printf("Il taxi [%ld] è arrivato a destinazione.\n", (long)getpid());
                            rispondi(&coda, msgid); //in base al successo/aborto/ecc, da modificare
                            
                        }
                        else{
                            EXIT_ON_ERROR
                        }  

                    //DEVO FARE I CONFRONTI E AGGIORNARE GLI ARRAY DEI RECORD!
                    printf("Celle percorse dal taxi: %d & Richieste raccolte: %d\n", infoTaxi.percorso, infoTaxi.richiesteRacc);
                    exit(EXIT_SUCCESS);
                   
                default: //è il padre
                    wait(NULL); //attende il figlio
                    sleep(3); //lascia al figlio il tempo di rispondere.
            
            } //fine fork taxi

        } //fine else
    //deallochiamo la coda
    if(msgctl(msgid, IPC_RMID, 0) == -1){
       EXIT_ON_ERROR
    }

    
exit(EXIT_SUCCESS);
}