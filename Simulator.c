#include "TaxiFunction.h"
//PROGRAMMA DI SIMULAZIONE CHE GENERA E INIZIALIZZA LA MAPPA (MESSA IN MEMORIA CONDIVISA), FORKA SO_SOURCES E GESTISCE LE RICHIESTE DI TAXI
//GESTIONE DEI NOSTRI SEGNALI CHE SCRIVONO RICHIESTE NELLA CODA
//GESTIONE DEI SEMAFORI PER LE CELLE OCCUPATE(?)


int main(){

    struct cella mappa[SO_HEIGHT][SO_WIDTH];
    int codaId;
    struct queue coda;
    mapGenerator(mappa);

    HolesGenerator(mappa);

    SourcesGenerator(mappa);

    TaxiGenerator(mappa);

    printMap(mappa);
    printf("i buchi sono: %d\n", SO_HOLES);
    printf("le sorgenti sono: %d\n", SO_SOURCES);
    printf("i taxi sono: %d\n", SO_TAXI); //se ne vedi di meno sulla mappa è perché ce ne sono di più in una cella

    //si attacca alla coda di messaggi
    if((codaId = msgget(MSGKEY, 0 ))==-1){ //mi aggancio alla coda (0 xk non mi interessano i flag)
        EXIT_ON_ERROR   
    } 
    //Vanno messi prima del controllo di SO_TAXI e SO_SOURCE perchè altrimenti Simulator termina prima di aver caricato nella coda
    //questi valori, che servono al MasterTaxi per capire se può eseguire o meno la simulazione.
    coda.numTaxi = SO_TAXI;
    coda.numSource = SO_SOURCES;

    //controllo che le sorgenti o i taxi non siano 0
    if((SO_SOURCES == 0) || (SO_TAXI == 0)){ //se il numero random ha dato 0, terminiamo la simulazione
        printf("Mi spiace ma al momento non ci sono richieste o taxi disponibili, fine simulazione.\n");
        //prima di effettuare la exit, faccio la msgsnd per far notare a MasterTaxi che o taxi o sorgenti sono 0
        if(msgsnd(codaId, &coda, sizeof(coda), IPC_NOWAIT)== -1){
                EXIT_ON_ERROR
        }
        //EXIT_ON_ERROR ----> LA EXIT_ON_ERROR NON VA BENE PERCHÈ NON VIENE GENERATO ALCUNO CODICE DI ERRORE E QUINDI NON FUNZIONA
        exit(EXIT_FAILURE);
    } 
    //forko una volta xk 1 SOURCE di prova, ma dovrò forkare SO_SOURCE volte!
    switch(fork()){

        case -1:
            EXIT_ON_ERROR

        case 0: //è il figlio (RICHIESTA TAXI)   
            sprintf(coda.msg, "La sorgente %ld richiama un taxi!", (long)getpid()); //questo però si può evitare volendo, oppure lo stampiamo 
            coda.mtype = (long) getpid();

            //le celle di partenza e di destinazione saranno random, ora le inizializzo
           // coda.partenza[0]= 1; //la partenza saranno le coordinate di della cella SOURCE che ha fatto la richiesta
          //coda.partenza[1]= 2;
            
            //aggiungo sulla coda di messaggi i valori di SO_TAXI e SO_SOURCE
            

            assignMat(mappa, coda.map); //assegna alla matrice della coda la matrice mappa

            getCellaArrivo(mappa, &coda); //metodo che mi dà random delle coordinate di arrivo e me le scrive nella coda di messaggi
            printf("arrivo è [%d][%d]\n", coda.arrivo[0], coda.arrivo[1]);

            //invio alla coda, DA RIVEDERE I BYTE CHE PASSO
            if(msgsnd(codaId, &coda, sizeof(coda), IPC_NOWAIT)== -1){
                EXIT_ON_ERROR
            }

            sleep(3);

            //riceve risposta dal taxi
            if(msgrcv(codaId, &coda, sizeof(coda), getpid(), 0) == -1) 
                EXIT_ON_ERROR

            printf("Risposta ricevuta! Il figlio del server mi ha scritto '%s'\n", coda.msg);   
             //6.Ricevuta la risposta termina.
            exit(EXIT_SUCCESS); 

        default: //è il padre che attende il figlio
            wait(NULL);
            sleep(2);

    }

//ricordarsi di mettere il for delle wait di tutti i figlio generati (le richieste taxi)

 /*    pure il taxi? (NON PER FORZA PROCESSO! si attacca alla coda di messaggi e rispondono con 1 delle 3 risposte a fine servizio (?))
    che deve avere le coordinate della cella (così quando queste coordinate sono uguali a quelle delle celle sorgenti la richiesta può servire la richiesta)
    (i taxi sono messi sempre in posizione casuale)

    struct taxi{
        int taxiCoord[2]; //dove si trova il taxi nella mappa
        int cellePercorse; //così quando ha finito il servizio scrive questo numero da qualche parte x tenere traccia di chi fa il viaggio più lungo
        int numRichieste; //idem sopra ma per sapere il taxi che ha avuto più richieste/clienti
    };
    
    il processo taxi che
    1. ha fatto piu` strada (numero di celle) di tutti
    2. ha fatto il viaggio piu` lungo (come tempo) nel servire una richiesta 
    3. ha raccolto piu` richieste/clienti


    -l'accesso ad ogni cella è un accesso alla sezione critica, regolata da semafori


*/        

}