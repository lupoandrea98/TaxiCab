#include "TaxiHeader.h"

//==============================================================================================================
//                                           FUNZIONI MASTER TAXI
//==============================================================================================================

void cercaTaxi(struct taxi *st, struct cella A[SO_HEIGHT][SO_WIDTH]){ 
//metodo che scorre tutta la mappa e mi restituisce un array di coordinate dove si trova un taxi
    int i,j;
    int flag = 1;
    
    for(i=0; i< SO_HEIGHT && flag; i++){
        for(j=0; j< SO_WIDTH && flag; j++){
            if((A[i][j].tipo ==3) || (A[i][j].tipo == 4)){
                st->coordTaxi[0] = i;
                st->coordTaxi[1] = j;
                flag = 0;
            }
        }
    }
}

void cercaSource(int coordSource[2], struct cella A[SO_HEIGHT][SO_WIDTH]){ 
//metodo che scorre tutta la mappa e mi restituisce un array di coordinate dove si trova un taxi
    int i,j;
    int flag = 1;
    
    for(i=0; i< SO_HEIGHT && flag; i++){
        for(j=0; j< SO_WIDTH && flag; j++){
            if((A[i][j].tipo == 2) || (A[i][j].tipo == 4)){
                coordSource[0] = i;
                coordSource[1] = j;
                flag = 0;
            }
        }
    }
}

//NON C'è BISOGNO DI DARE LA RISPOSTA (OPPURE Sì MA SCRIVIAMO SOLO PRINTF "COMPLETATA", "INEVASA", "ABORTITA" GIUSTO PER CAPIRE NOI),
//INCREMENTIAMO I RISULTATI QUI PERCHè SARà QUESTO IL PROCESSO MASTER CHE CONSERVA I DATI, QUINDI IL PROCESSO TAXI PRIMA DI FARE L'EXIT 
void rispondi(const struct queue* richiesta, int msgid) {
    struct queue risposta; //nuova struttura coda simmetrica

    sprintf(risposta.msg, "Sono il taxi [pid: %ld], richiesta servita :) ", (long)getpid());
    risposta.mtype = (*richiesta).mtype; //e come tipo metto il pid del figlio che ha fatto la domanda
     //invio:
    if(msgsnd(msgid, &risposta, (sizeof(struct queue)-sizeof(long)-sizeof(int[2])*2-sizeof(struct cella)*SO_WIDTH*SO_HEIGHT), IPC_NOWAIT)== -1){ 
    EXIT_ON_ERROR
    }
     // attendo qualche secondo
    sleep(2);
}

//DA MODIFICARE, IMPLEMENTARLO CON IL TEMPO E CON IL NUMERO DI SUCCESSI/ABORTITI/ECC...
int TaxiMover(struct cella mappa[SO_HEIGHT][SO_WIDTH], struct taxi *st, int arrivo_r, int arrivo_c){

    int flagR = 1;
    int flagC = 1;
    int finalflag=1;
    int taxi_r = st->coordTaxi[0];
    int taxi_c = st->coordTaxi[1];

    printf("Partenza [%d][%d]\n", taxi_r, taxi_c);
    printf("Arrivo [%d][%d]\n", arrivo_r, arrivo_c);

    /*Cose da aggiungere:
        - Tempo di attraversamento delle celle.
        - Aggiungere le nanosleep per il tempo di attraversamento delle celle. 
        - 
    */                                                      

    while(finalflag){

        while(taxi_r<SO_HEIGHT && flagR){   //Righe

            while(taxi_c<SO_WIDTH && flagC){    //Colonne
                
                if((taxi_c < arrivo_c) && (mappa[taxi_r][taxi_c + 1].tipo != 1)){
                    //Qui va aggiunta la nanosleep
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                        EXIT_ON_ERROR
                    taxi_c++;
                    printf("Mi sposto a dx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                        //implementare capacità, semafori, ecc...
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        //.....
                }else if((taxi_c > arrivo_c)  && (mappa[taxi_r][taxi_c - 1].tipo != 1)){
                    //Qui va aggiunta la nanosleep
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                        EXIT_ON_ERROR
                    taxi_c--;
                    printf("Mi sposto a sx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                        //implementare capacità, semafori, ecc...
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        //.....
                }else if(taxi_c == arrivo_c){
                    //printf("Ok sono nella stessa colonna.\n");
                    flagC = 0;
                    
                }else{  //Ostacolo

                    if(taxi_r + 1 < SO_HEIGHT){
                        //Qui va aggiunta la nanosleep
                        printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                        if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                            EXIT_ON_ERROR
                        taxi_r++;
                        printf("Ostacolo, vado in basso: TAXI[%d][%d]\n", taxi_r, taxi_c);
                         //implementare capacità, semafori, ecc...
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        //..... 
                        
                    }else if(taxi_r - 1 >= 0){
                        //Qui va aggiunta la nanosleep
                        printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                        if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                            EXIT_ON_ERROR
                        taxi_r--;
                        printf("Ostacolo, vado in alto: TAXI[%d][%d]\n", taxi_r, taxi_c);
                          //implementare capacità, semafori, ecc...
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        //.....                    
                    }
                }

            }   //Fine while colonne
        
            if((taxi_r < arrivo_r)  && (mappa[taxi_r + 1][taxi_c].tipo != 1)){

                //Qui va aggiunta la nanosleep
                printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                    EXIT_ON_ERROR
                taxi_r++;
                printf("Mi sposto in basso: TAXI[%d][%d]\n", taxi_r, taxi_c);
                //implementare capacità, semafori, ecc...
                mappa[taxi_r][taxi_c].numTaxiPresenti++;
                mappa[taxi_r][taxi_c].numVolteAttr++;
  		        st->coordTaxi[0] = taxi_r;
                st->coordTaxi[1] = taxi_c;
                st->percorso++;
                //..... 

            }else if((taxi_r > arrivo_r)  && (mappa[taxi_r - 1][taxi_c].tipo != 1)){
                
                printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                    EXIT_ON_ERROR
                taxi_r--;
                printf("Mi sposto in alto: TAXI[%d][%d]\n", taxi_r, taxi_c);
                //implementare capacità, semafori, ecc...
                mappa[taxi_r][taxi_c].numTaxiPresenti++;
                mappa[taxi_r][taxi_c].numVolteAttr++;
  		        st->coordTaxi[0] = taxi_r;
                st->coordTaxi[1] = taxi_c;
                st->percorso++;
                //..... 

            }else if(taxi_r == arrivo_r){

                //printf("Ok sono nella stessa riga.\n");
                flagR = 0;
                break; //non c'è bisogno?

            }else{  //Ostacolo
                   
                if(taxi_c + 1 < SO_WIDTH){
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                        EXIT_ON_ERROR
                    taxi_c++;
                    printf("Ostacolo, vado a dx: TAXI[%d][%d]\n", taxi_r, taxi_c); 
                    //implementare capacità, semafori, ecc...
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    //.....  

                }else if(taxi_c - 1 >= 0){
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1)
                        EXIT_ON_ERROR
                    taxi_c--;
                    printf("Ostacolo, vado a sx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                    //implementare capacità, semafori, ecc...
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    //..... 
                }
            }
        }   //Fine while righe

        if(taxi_r == arrivo_r && taxi_c == arrivo_c){

            printf("Sono arrivato a destinazione.\n");
            //implementare capacità, semafori, ecc...
            mappa[taxi_r][taxi_c].numTaxiPresenti++;
            mappa[taxi_r][taxi_c].numVolteAttr++;
            st->coordTaxi[0] = taxi_r;
            st->coordTaxi[1] = taxi_c;
                        
            //..... 
            finalflag = 0; //per uscire dal blocco totale
            return 1;

       }else{
           //il taxi non è ancora arrivato allora ripete il ciclo:
            printf("Sto per arrivare...\n");
            sleep(1);
            flagR = 1; //si risettano così si ripete
            flagC = 1;      
        }

    }//fine del blocco
    return 0;
} //ritorna 1 in caso di successo (arrivo a destinazione) 0 in caso di fallimento



//==============================================================================================================
//                                          FUNZIONI SIMULATOR
//==============================================================================================================

void mapGenerator(struct cella map[SO_HEIGHT][SO_WIDTH]){
    int i,j; //riga e colonna 
    int SO_CAP_MIN; //num minimo taxi ospitati in una cella
    int SO_CAP_MAX; //num massimo taxi ospitati in una cella
    unsigned long SO_TIMENSEC_MIN; //tempo minimo attraversamento cella (nanosec)
    unsigned long SO_TIMENSEC_MAX; //tempo massimo attraversamento cella (nanosec)

    //definisco i valori random
    srand(time(NULL));
    SO_CAP_MAX = rand() % 20; //num casuale tra 0 e RAND_MAX (as defined by the header file <stdlib.h>).
    SO_CAP_MIN = rand() %(SO_CAP_MAX+1); //ovviamente deve essere inferiore o uguale al max, metto il +1 includere la possibilità di averlo uguale
    SO_TIMENSEC_MAX = rand() % 1000; 
    SO_TIMENSEC_MIN = rand() %(SO_TIMENSEC_MAX+1);
    
    //per avere un numero random compreso in un itervallo: rand() % (upper - lower + 1)) + lower; 
    //creazione mappa inizializzata a 0:
    for(i=0; i<SO_HEIGHT; i++){
        for(j=0; j<SO_WIDTH; j++){
            map[i][j].tipo = 0;
            map[i][j].carattere = '0';
            map[i][j].cellaCoord[0] = i;
            map[i][j].cellaCoord[1] = j;
            //da implementare poi nel taximover:
            map[i][j].numTaxiPresenti = 0;  //DEVE ESSERE SEMPRE MINORE/UGUALE DI capMax
            map[i][j].tempAttravers.tv_sec = 1;
            map[i][j].tempAttravers.tv_nsec = (rand() % (SO_TIMENSEC_MAX - SO_TIMENSEC_MIN +1)) + SO_TIMENSEC_MIN; //numero random tra l'intervallo
            map[i][j].numVolteAttr = 0;
            map[i][j].capMax = (rand() % (SO_CAP_MAX - SO_CAP_MIN +1)) + SO_CAP_MIN;    //Numero massimo di taxi che la cella può contenere
            //printf("Cella[%d][%d] ha capacità %d\n", i, j, map[i][j].capMax);
        }
    }
}

void printMap(struct cella map[SO_HEIGHT][SO_WIDTH]){
    puts("\n");

    int i,j; //riga e colonna
    for(i=0; i<SO_HEIGHT; i++){
        for(j=0; j<SO_WIDTH; j++){
            if(map[i][j].tipo == 1)
                printf(BOLDRED "%c\t" RESET, map[i][j].carattere); //mi stampa gli X buchi rossi 
            else if(map[i][j].tipo == 2) 
               printf(GREEN "%c\t" RESET, map[i][j].carattere); //mi stampa le S sorgenti verdi
            else if(map[i][j].tipo == 3) 
                printf(CYAN "%c\t" RESET, map[i][j].carattere); //se sono taxi stampa * azzurri
            else if(map[i][j].tipo == 4)
                printf(GREEN "%c\t" RESET, map[i][j].carattere); //se sono taxi su sorgenti li stampa in verde
            else
                printf("%c\t", map[i][j].carattere); //altrimenti mi stampa gli zeri, celle vuote
        }         
    puts("\n");
    } 
puts("\n");    
}

int controlloBuchi(struct cella A[SO_HEIGHT][SO_WIDTH], int i, int j){

    int R = SO_HEIGHT;
    int C = SO_WIDTH;
    
    if(i == 0 && j ==0){ //ALTO SX

		if((A[i][j].tipo!= 1) && (A[i][j+1].tipo!= 1) && (A[i+1][j].tipo!= 1) && (A[i+1][j+1].tipo!= 1)){
			return 1;
		
		}		
				
	}else if(i > 0 && j > 0 && i<(R-1) && j<(C-1)){	//CENTRALE esclusi gli estremi

		if((A[i][j].tipo!=1) && (A[i-1][j-1].tipo!=1) && (A[i-1][j].tipo!=1) && (A[i-1][j+1].tipo!=1) && (A[i][j-1].tipo!=1) && (A[i][j+1].tipo!=1) && (A[i+1][j-1].tipo!=1) && (A[i+1][j].tipo!=1) &&(A[i+1][j+1].tipo!=1)){
			return 1;
					
		}

	}else if(i == 0 && j>0 && j<(C-1)){	//ALTO centro

		if((A[i][j].tipo!= 1) && (A[i][j-1].tipo!= 1) && (A[i][j+1].tipo!= 1) && (A[i+1][j-1].tipo!= 1) && (A[i+1][j].tipo!= 1)&& (A[i+1][j+1].tipo!= 1)){
			return 1;
					
		}	

	}else if(i == (R-1) && j>0 && j<(C-1)){	//BASSO centro

		if((A[i][j].tipo!= 1) && (A[i][j-1].tipo!= 1) && (A[i][j+1].tipo!= 1) && (A[i-1][j-1].tipo!= 1) && (A[i-1][j].tipo!= 1)&& (A[i-1][j+1].tipo!= 1)){
			return 1;
					
		}	

	}else if(i>0 && i<(R-1)  && j==0){	//SX centro

		if((A[i][j].tipo!= 1) && (A[i-1][j].tipo!= 1) && (A[i+1][j].tipo!= 1) && (A[i-1][j+1].tipo!= 1) && (A[i][j+1].tipo!= 1)&& (A[i+1][j+1].tipo!= 1)){
			return 1;
					
		}		

	}else if(i>0 && i<(R-1)  && j==(C-1)){	//DX centro

		if((A[i][j].tipo!= 1) && (A[i-1][j].tipo!= 1) && (A[i+1][j].tipo!= 1) && (A[i-1][j-1].tipo!= 1) && (A[i][j-1].tipo!= 1)&& (A[i+1][j-1].tipo!= 1)){
			return 1;
					
		}			
				

	}else if(i == 0 && j == (C-1)){	//ALTO DX

		if((A[i][j].tipo!= 1) && (A[i][j-1].tipo!= 1) && (A[i+1][j].tipo!= 1) && (A[i+1][j-1].tipo!= 1)){
			return 1;
					
		}

	}else if(i == (R-1) && j == 0){ //BASSO SX

		if((A[i][j].tipo!= 1) && (A[i-1][j].tipo!= 1) && (A[i][j+1].tipo!= 1) && (A[i-1][j+1].tipo!= 1)){
			return 1;
					
		}

	}else if(i == (R-1) && j == (C-1)){ //BASSO DX

		if((A[i][j].tipo!= 1) && (A[i-1][j].tipo!= 1) && (A[i][j-1].tipo!= 1) && (A[i-1][j-1].tipo!= 1)){
			return 1;
					
		}
	}
    
    return 0; 
            
}

void HolesGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]){

    int numHoles, r, c;

    //srand(time(NULL));
    //Genero un numero random di buchi da creare nella mappa.
    numHoles = rand() %(SO_HEIGHT*SO_WIDTH-1);

    //printf("i buchi dovevano essere: %d\n", numHoles);

    while(numHoles!=0){
        //Genero coordinate randomiche
        c = rand() %SO_WIDTH; 
        r = rand() %SO_HEIGHT;

        //Controllo che le coordinate generate rispettino i requisiti per generare un buco.
        if(controlloBuchi(A, r, c)){
            //printf("riga: %d\n", r);
            //printf("colonna: %d\n", c);
            A[r][c].tipo = 1;
            A[r][c].carattere = 'X';
            A[r][c].tempAttravers.tv_nsec = 0; //potevo lasciare anche invariato ma così è più preciso
           //il resto lascio invariato, già inizializzato.  
            SO_HOLES++;     
        }
        else{
            EXIT_ON_ERROR
        }

        numHoles--;   
    }   

}

void SourcesGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]){

    int numSources, r, c;

    //srand(time(NULL));
    //Genero un numero random di buchi da creare nella mappa.
    numSources = rand() %(SO_HEIGHT*SO_WIDTH-SO_HOLES);
    printf("Num originale richieste: %d\n", numSources);
    //printf("le sorgenti dovevano essere: %d\n", numSources);
    
    while(numSources!=0){
        //Genero coordinate randomiche
        c = rand() %SO_WIDTH; 
        r = rand() %SO_HEIGHT;
        //printf("riga: %d\n", r);
        //printf("colonna: %d\n", c);

        //Controllo che le sorgenti generate rispettino i requisiti (non devono sovrapporsi)
        if((A[r][c].tipo!= 1) && (A[r][c].tipo != 2)){ //"2" indica sulla mappa la posizione della cella sorgente
           A[r][c].tipo = 2;
           A[r][c].carattere = 'S';
           SO_SOURCES++; 
        }
    
    	numSources--;   
    	
    } 
 
}


//NOTA: Nella funzione TaxiGenerator dobbiamo aggiungere un controllo per la capacità della cella dove 
//viene creato per evitare un sovra-affollamento delle celle.
void TaxiGenerator(struct cella A[SO_HEIGHT][SO_WIDTH]){

    int numTaxi, r, c;

    //srand(time(NULL));
    //Genero un numero random di taxi da creare nella mappa.
    numTaxi = rand() %(SO_HEIGHT*SO_WIDTH-SO_HOLES); //è giusto? deve essere proporzionato, o può superare righe*colonne?
    printf("Num originale taxi: %d\n", numTaxi);
    while(numTaxi!=0){
        //Genero coordinate randomiche
        c = rand() %SO_WIDTH; 
        r = rand() %SO_HEIGHT;
        //printf("riga: %d\n", r);
        //printf("colonna: %d\n", c);

        //controllo che ci sia spazio nella cella per il nuovo taxi
        //DOBBIAMO METTERE IL SEMAFORO? 
        if(A[r][c].numTaxiPresenti < A[r][c].capMax){
            //Controllo che le sorgenti generate rispettino i requisiti (non devono sovrapporsi)
            if((A[r][c].tipo!= 1)){ //cella non bloccata!
                A[r][c].carattere = '*';
                A[r][c].numVolteAttr++;
                A[r][c].numTaxiPresenti++; 
                if(A[r][c].tipo == 0){ 
                    A[r][c].tipo = 3; //è una cella vuota e diventa taxi
                }else{    
                    A[r][c].tipo = 4; //è una cella che è contiene sia un taxi che una sorgente
                }         
                SO_TAXI++;  
            }
        }

    	numTaxi--;       	
    }  

}

void assignMat(struct cella A[SO_HEIGHT][SO_WIDTH], struct cella B[SO_HEIGHT][SO_WIDTH]){ //assegna matrice

    int i,j;

    for(i=0;i<SO_HEIGHT;i++)
        for(j=0;j<SO_WIDTH;j++){
            B[i][j].tipo = A[i][j].tipo;
            B[i][j].carattere = A[i][j].carattere;
            B[i][j].cellaCoord[0] = A[i][j].cellaCoord[0];
            B[i][j].cellaCoord[1]= A[i][j].cellaCoord[1];
            B[i][j].numTaxiPresenti = A[i][j].numTaxiPresenti;
            B[i][j].tempAttravers = A[i][j].tempAttravers;
            B[i][j].numVolteAttr = A[i][j].numVolteAttr;
        }
}

//Metodo che mi dà delle coordinate a caso di arrivo
void getCellaArrivo(struct cella A[SO_HEIGHT][SO_WIDTH], struct queue *coda){ //passo la struttura per puntatore così non perdo dati

   int r,c;
   int flag = 1;

   while(flag){
       r = rand() %SO_HEIGHT;
       c = rand() %SO_WIDTH; 

       if(A[r][c].tipo!=1){
           coda->arrivo[0] = r;
           coda->arrivo[1] = c;
           flag = 0;     
       }  
    }       
}
