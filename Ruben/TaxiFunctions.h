#include "TaxiHeader.h"

//==============================================================================================================
//                                           FUNZIONI TAXI
//==============================================================================================================
void reset(){               //Alternativa alla macro RESET presente nell'header
    printf("\033[0m");
}
void calcolaMax(struct data* pt , struct taxi *taxi){
  
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
      pt->richPiuRaccolte[1] = taxi->pid;
  }
       
}

void cercaSource(struct source *st, struct cella A[SO_HEIGHT][SO_WIDTH]){ 
//metodo che scorre tutta la mappa e mi restituisce un array di coordinate dove si trova un taxi
    int r,c;
    int flag = 1;

    while(flag){
        r = rand() %SO_HEIGHT;
        c = rand() %SO_WIDTH; 

        if((A[r][c].tipo == 2) || (A[r][c].tipo == 4)){
            st->coordSource[0] = r;
            st->coordSource[1] = c;
            flag = 0;
        }
    }
}

//NON C'è BISOGNO DI DARE LA RISPOSTA (OPPURE Sì MA SCRIVIAMO SOLO PRINTF "COMPLETATA", "INEVASA", "ABORTITA" GIUSTO PER CAPIRE NOI),
//INCREMENTIAMO I RISULTATI QUI PERCHè SARà QUESTO IL PROCESSO MASTER CHE CONSERVA I DATI, QUINDI IL PROCESSO TAXI PRIMA DI FARE L'EXIT 
void rispondi(const struct queue* richiesta, int msgid, int valid) {

    struct queue risposta; //nuova struttura coda simmetrica
    if(valid == 1){
        sprintf(risposta.msg, "Sono il taxi [pid: %ld], richiesta servita :) ", (long)getpid());
    }else{
        sprintf(risposta.msg, "Sono il Taxi[pid: %ld], richiesta non servita :( ", (long)getpid()); 
    }

    risposta.mtype = (*richiesta).mtype; //e come tipo metto il pid del figlio che ha fatto la domanda
    //invio:
    if(msgsnd(msgid, &risposta, (sizeof(struct queue)-sizeof(long)-sizeof(int[2])), IPC_NOWAIT)== -1){ 
        EXIT_ON_ERROR
    }

        // attendo qualche secondo
        sleep(2);
       
}

//DA MODIFICARE, IMPLEMENTARLO CON IL TEMPO E CON IL NUMERO DI SUCCESSI/ABORTITI/ECC...
int TaxiMover(struct cella mappa[SO_HEIGHT][SO_WIDTH], struct taxi *st, int arrivo_r, int arrivo_c, long *tempoImpiegato){

    if(*tempoImpiegato > SO_TIMEOUT)
        return 0;    

    int flagR = 1;
    int flagC = 1;
    int finalflag=1;
    int taxi_r = st->coordTaxi[0];
    int taxi_c = st->coordTaxi[1];
    int valid = 1;      //Flag che controlla il tempo impiegato dal taxi per esaudire la richiesta (Deve essere minore di SO_TIMEOUT)

    printf("Partenza [%d][%d]\n", taxi_r, taxi_c);
    printf("Arrivo [%d][%d]\n", arrivo_r, arrivo_c); 
                                                   
    while(finalflag){
        
        while(taxi_r<SO_HEIGHT && flagR){   //Righe
    
            while(taxi_c<SO_WIDTH && flagC){    //Colonne
                
                if((taxi_c < arrivo_c) && (mappa[taxi_r][taxi_c + 1].tipo != 1 )){
                    if(reserveSem(semid, 0) == -1){     //Decremento semaforo.
                        EXIT_ON_ERROR
                    }  
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                        EXIT_ON_ERROR
                    }
                    taxi_c++;
                    printf("Mi sposto a dx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                    //implementare capacità, semafori, ecc...
                    
                    mappa[taxi_r][taxi_c-1].numTaxiPresenti--;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    //Modifiche ai dati della mappa per la stampa
                    if(mappa[taxi_r][taxi_c-1].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c-1].tipo == 3){          //CELLA SOLO DI TAXI
                            mappa[taxi_r][taxi_c-1].tipo = 0;
                            mappa[taxi_r][taxi_c-1].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c-1].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                            mappa[taxi_r][taxi_c-1].tipo = 2;
                            mappa[taxi_r][taxi_c-1].carattere = 'S';
                        }
                    }
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                            mappa[taxi_r][taxi_c].tipo = 3;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                            mappa[taxi_r][taxi_c].tipo = 4;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }
                    }
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                
                    *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                    if(*tempoImpiegato > SO_TIMEOUT){
                        mappa[taxi_r][taxi_c].numTaxiPresenti--;
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                                mappa[taxi_r][taxi_c].tipo = 0;
                                mappa[taxi_r][taxi_c].carattere = '0';
                            }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                                mappa[taxi_r][taxi_c].tipo = 2;
                                mappa[taxi_r][taxi_c].carattere = 'S';
                            }
                        }
                        printf("Richiesta non soddisfatta nei tempi previsti\n");
                        if(releaseSem(semid, 0) == -1){    //Incremento semaforo.
                            EXIT_ON_ERROR
                        }
                        return 0;
                    }    
                //SEMAFORO OUT
                    if(releaseSem(semid, 0) == -1){    //Incremento semaforo.
                        EXIT_ON_ERROR
                    }
                }else if((taxi_c > arrivo_c)  && (mappa[taxi_r][taxi_c - 1].tipo != 1)){
                    if(reserveSem(semid, 1) == -1){     //Decremento semaforo.
                        EXIT_ON_ERROR
                    }
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                        EXIT_ON_ERROR
                    }
                    taxi_c--;
                    printf("Mi sposto a sx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                        //implementare capacità, semafori, ecc...
                        
                    mappa[taxi_r][taxi_c+1].numTaxiPresenti--;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    
                    if(mappa[taxi_r][taxi_c+1].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c+1].tipo == 3){
                            mappa[taxi_r][taxi_c+1].tipo = 0;
                            mappa[taxi_r][taxi_c+1].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c+1].tipo == 4){
                            mappa[taxi_r][taxi_c+1].tipo = 2;
                            mappa[taxi_r][taxi_c+1].carattere = 'S';
                        }
                    }
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                            mappa[taxi_r][taxi_c].tipo = 3;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                            mappa[taxi_r][taxi_c].tipo = 4;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }
                    }
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                    //.....
                    *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                    if(*tempoImpiegato > SO_TIMEOUT){
                        mappa[taxi_r][taxi_c].numTaxiPresenti--;
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                                mappa[taxi_r][taxi_c].tipo = 0;
                                mappa[taxi_r][taxi_c].carattere = '0';
                            }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                                mappa[taxi_r][taxi_c].tipo = 2;
                                mappa[taxi_r][taxi_c].carattere = 'S';
                            }
                        }
                        printf("Richiesta non soddisfatta nei tempi previsti\n");
                        if(releaseSem(semid, 1) == -1){     //Decremento semaforo.
                            EXIT_ON_ERROR
                        }
                        return 0;
                    }
                    if(releaseSem(semid, 1) == -1){     //Decremento semaforo.
                        EXIT_ON_ERROR
                    }
                }else if(taxi_c == arrivo_c){
                    //Sono nella stessa colonna
                    flagC = 0;
                    
                }else{  //Ostacolo
                    
                    if(taxi_r + 1 < SO_HEIGHT){
                        if(reserveSem(semid, 2) == -1){     //Decremento semaforo.
                            EXIT_ON_ERROR
                        }
                        printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                        if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                            EXIT_ON_ERROR
                        }
                        taxi_r++;
                        printf("Ostacolo, vado in basso: TAXI[%d][%d]\n", taxi_r, taxi_c);
                         //implementare capacità, semafori, ecc...
                        
                        mappa[taxi_r-1][taxi_c].numTaxiPresenti--;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        if(mappa[taxi_r-1][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r-1][taxi_c].tipo == 3){
                                mappa[taxi_r-1][taxi_c].tipo = 0;
                                mappa[taxi_r-1][taxi_c].carattere = '0';
                            }else if(mappa[taxi_r-1][taxi_c].tipo == 4){
                                mappa[taxi_r-1][taxi_c].tipo = 2;
                                mappa[taxi_r-1][taxi_c].carattere = 'S';
                            }
                        }
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                                mappa[taxi_r][taxi_c].tipo = 3;
                                mappa[taxi_r][taxi_c].carattere = '*';
                            }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                                mappa[taxi_r][taxi_c].tipo = 4;
                                mappa[taxi_r][taxi_c].carattere = '*';
                            }
                        }
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        //..... 
                        *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                        if(*tempoImpiegato > SO_TIMEOUT){
                            mappa[taxi_r][taxi_c].numTaxiPresenti--;
                            if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                                if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                                    mappa[taxi_r][taxi_c].tipo = 0;
                                    mappa[taxi_r][taxi_c].carattere = '0';
                                }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                                    mappa[taxi_r][taxi_c].tipo = 2;
                                    mappa[taxi_r][taxi_c].carattere = 'S';
                                }
                            }
                            printf("Richiesta non soddisfatta nei tempi previsti\n");
                            if(releaseSem(semid, 2) == -1){    //Incremento semaforo.
                                EXIT_ON_ERROR
                            }
                            return 0;
                        }
                        if(releaseSem(semid, 2) == -1){    //Incremento semaforo.
                            EXIT_ON_ERROR
                        }
                    }else if(taxi_r - 1 >= 0){
                        if(reserveSem(semid, 3) == -1){     //Decremento semaforo.
                            EXIT_ON_ERROR
                        }
                        printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                        if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                            EXIT_ON_ERROR
                        }
                        taxi_r--;
                        printf("Ostacolo, vado in alto: TAXI[%d][%d]\n", taxi_r, taxi_c);
                          //implementare capacità, semafori, ecc...
                        
                        mappa[taxi_r+1][taxi_c].numTaxiPresenti--;
                        mappa[taxi_r][taxi_c].numVolteAttr++;
                        st->coordTaxi[0] = taxi_r;
                        st->coordTaxi[1] = taxi_c;
                        st->percorso++;
                        if(mappa[taxi_r+1][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r+1][taxi_c].tipo == 3){
                                mappa[taxi_r+1][taxi_c].tipo = 0;
                                mappa[taxi_r+1][taxi_c].carattere = '0';
                            }else if(mappa[taxi_r+1][taxi_c].tipo == 4){
                                mappa[taxi_r+1][taxi_c].tipo = 2;
                                mappa[taxi_r+1][taxi_c].carattere = 'S';
                            }
                        }
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                                mappa[taxi_r][taxi_c].tipo = 3;
                                mappa[taxi_r][taxi_c].carattere = '*';
                            }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                                mappa[taxi_r][taxi_c].tipo = 4;
                                mappa[taxi_r][taxi_c].carattere = '*';
                            }
                        }
                        mappa[taxi_r][taxi_c].numTaxiPresenti++;
                        //.....   
                        *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                        if(*tempoImpiegato > SO_TIMEOUT){
                            mappa[taxi_r][taxi_c].numTaxiPresenti--;
                            if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                                if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                                    mappa[taxi_r][taxi_c].tipo = 0;
                                    mappa[taxi_r][taxi_c].carattere = '0';
                                }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                                    mappa[taxi_r][taxi_c].tipo = 2;
                                    mappa[taxi_r][taxi_c].carattere = 'S';
                                }
                            }
                            printf("Richiesta non soddisfatta nei tempi previsti\n");
                            if(releaseSem(semid, 3) == -1){    //Incremento semaforo.
                                EXIT_ON_ERROR
                            } 
                            return 0;
                        }
                        if(releaseSem(semid, 3) == -1){    //Incremento semaforo.
                            EXIT_ON_ERROR
                        }                
                    }
                }

            }   //Fine while colonne
            
            if((taxi_r < arrivo_r)  && (mappa[taxi_r + 1][taxi_c].tipo != 1)){
                if(reserveSem(semid, 4) == -1){     //Decremento semaforo.
                    EXIT_ON_ERROR
                }
                //Qui va aggiunta la nanosleep
                printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                    EXIT_ON_ERROR
                }
                taxi_r++;
                printf("Mi sposto in basso: TAXI[%d][%d]\n", taxi_r, taxi_c);
                //implementare capacità, semafori, ecc...
                
                mappa[taxi_r-1][taxi_c].numTaxiPresenti--;
                mappa[taxi_r][taxi_c].numVolteAttr++;
  		        st->coordTaxi[0] = taxi_r;
                st->coordTaxi[1] = taxi_c;
                st->percorso++;
                if(mappa[taxi_r-1][taxi_c].numTaxiPresenti == 0){
                    if(mappa[taxi_r-1][taxi_c].tipo == 3){
                        mappa[taxi_r-1][taxi_c].tipo = 0;
                        mappa[taxi_r-1][taxi_c].carattere = '0';
                    }else if(mappa[taxi_r-1][taxi_c].tipo == 4){
                        mappa[taxi_r-1][taxi_c].tipo = 2;
                        mappa[taxi_r-1][taxi_c].carattere = 'S';
                    }
                }
                if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                    if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                        mappa[taxi_r][taxi_c].tipo = 3;
                        mappa[taxi_r][taxi_c].carattere = '*';
                    }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                        mappa[taxi_r][taxi_c].tipo = 4;
                        mappa[taxi_r][taxi_c].carattere = '*';
                    }
                    
                }
                mappa[taxi_r][taxi_c].numTaxiPresenti++;
                //..... 
                *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                if(*tempoImpiegato > SO_TIMEOUT){
                    mappa[taxi_r][taxi_c].numTaxiPresenti--;
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                            mappa[taxi_r][taxi_c].tipo = 0;
                            mappa[taxi_r][taxi_c].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                            mappa[taxi_r][taxi_c].tipo = 2;
                            mappa[taxi_r][taxi_c].carattere = 'S';
                        }
                    }
                    printf("Richiesta non soddisfatta nei tempi previsti\n");
                    if(releaseSem(semid, 4) == -1){    //Incremento semaforo.
                        EXIT_ON_ERROR
                    } 
                    return 0;
                }
                if(releaseSem(semid, 4) == -1){    //Incremento semaforo.
                    EXIT_ON_ERROR
                } 
            
            }else if((taxi_r > arrivo_r)  && (mappa[taxi_r - 1][taxi_c].tipo != 1)){
                if(reserveSem(semid, 5) == -1){     //Decremento semaforo.
                    EXIT_ON_ERROR
                }
                printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                    EXIT_ON_ERROR
                }
                taxi_r--;
                printf("Mi sposto in alto: TAXI[%d][%d]\n", taxi_r, taxi_c);
                //implementare capacità, semafori, ecc...
                
                mappa[taxi_r+1][taxi_c].numTaxiPresenti--;
                mappa[taxi_r][taxi_c].numVolteAttr++;
  		        st->coordTaxi[0] = taxi_r;
                st->coordTaxi[1] = taxi_c;
                st->percorso++;
                if(mappa[taxi_r+1][taxi_c].numTaxiPresenti == 0){
                    if(mappa[taxi_r+1][taxi_c].tipo == 3){
                        mappa[taxi_r+1][taxi_c].tipo = 0;
                        mappa[taxi_r+1][taxi_c].carattere = '0';
                    }else if(mappa[taxi_r+1][taxi_c].tipo == 4){
                        mappa[taxi_r+1][taxi_c].tipo = 2;
                        mappa[taxi_r+1][taxi_c].carattere = 'S';
                    }
                }
                if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                    if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                        mappa[taxi_r][taxi_c].tipo = 3;
                        mappa[taxi_r][taxi_c].carattere = '*';
                    }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                        mappa[taxi_r][taxi_c].tipo = 4;
                        mappa[taxi_r][taxi_c].carattere = '*';
                    }
                }
                mappa[taxi_r][taxi_c].numTaxiPresenti++;
                //.....
                *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                if(*tempoImpiegato > SO_TIMEOUT){
                    mappa[taxi_r][taxi_c].numTaxiPresenti--;
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                            mappa[taxi_r][taxi_c].tipo = 0;
                            mappa[taxi_r][taxi_c].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                            mappa[taxi_r][taxi_c].tipo = 2;
                            mappa[taxi_r][taxi_c].carattere = 'S';
                        }
                    }
                    printf("Richiesta non soddisfatta nei tempi previsti\n");
                    if(releaseSem(semid, 5) == -1){    //Incremento semaforo.
                        EXIT_ON_ERROR
                    } 
                    return 0;
                }
                if(releaseSem(semid, 5) == -1){    //Incremento semaforo.
                    EXIT_ON_ERROR
                } 

            }else if(taxi_r == arrivo_r){

                //Sono nella stessa riga
                flagR = 0;
                break; //non c'è bisogno?
            
            }else{  //Ostacolo
                
                if(taxi_c + 1 < SO_WIDTH){
                    if(reserveSem(semid, 6) == -1){     //Decremento semaforo.
                        EXIT_ON_ERROR
                    }   
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                        EXIT_ON_ERROR
                    }
                    taxi_c++;
                    printf("Ostacolo, vado a dx: TAXI[%d][%d]\n", taxi_r, taxi_c); 
                    //implementare capacità, semafori, ecc...
                    
                    mappa[taxi_r][taxi_c-1].numTaxiPresenti--;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    if(mappa[taxi_r][taxi_c-1].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c-1].tipo == 3){
                            mappa[taxi_r][taxi_c-1].tipo = 0;
                            mappa[taxi_r][taxi_c-1].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c-1].tipo == 4){
                            mappa[taxi_r][taxi_c-1].tipo = 2;
                            mappa[taxi_r][taxi_c-1].carattere = 'S';
                        }
                    }
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                            mappa[taxi_r][taxi_c].tipo = 3;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                            mappa[taxi_r][taxi_c].tipo = 4;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }
                    }
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                    //..... 
                    *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                    if(*tempoImpiegato > SO_TIMEOUT){
                        mappa[taxi_r][taxi_c].numTaxiPresenti--;
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                            mappa[taxi_r][taxi_c].tipo = 0;
                            mappa[taxi_r][taxi_c].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                            mappa[taxi_r][taxi_c].tipo = 2;
                            mappa[taxi_r][taxi_c].carattere = 'S';
                        }
                    }
                        printf("Richiesta non soddisfatta nei tempi previsti\n");
                        if(releaseSem(semid, 6) == -1){    //Incremento semaforo.
                            EXIT_ON_ERROR
                        } 
                        return 0;
                    }
                    if(releaseSem(semid, 6) == -1){    //Incremento semaforo.
                        EXIT_ON_ERROR
                    } 
                   
                }else if(taxi_c - 1 >= 0){
                    if(reserveSem(semid, 7) == -1){     //Decremento semaforo.
                        EXIT_ON_ERROR
                    }
                    printf("cella[%d][%d].tempoattraversamento = %ld  ",taxi_r, taxi_c, mappa[taxi_r][taxi_c].tempAttravers.tv_nsec);
                    if(nanosleep(&mappa[taxi_r][taxi_c].tempAttravers, NULL) == -1){
                        EXIT_ON_ERROR
                    }
                    taxi_c--;
                    printf("Ostacolo, vado a sx: TAXI[%d][%d]\n", taxi_r, taxi_c);
                    //implementare capacità, semafori, ecc...
                    
                    mappa[taxi_r][taxi_c+1].numTaxiPresenti--;
                    mappa[taxi_r][taxi_c].numVolteAttr++;
                    st->coordTaxi[0] = taxi_r;
                    st->coordTaxi[1] = taxi_c;
                    st->percorso++;
                    if(mappa[taxi_r][taxi_c+1].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c+1].tipo == 3){
                            mappa[taxi_r][taxi_c+1].tipo = 0;
                            mappa[taxi_r][taxi_c+1].carattere = '0';
                        }else if(mappa[taxi_r][taxi_c+1].tipo == 4){
                            mappa[taxi_r][taxi_c+1].tipo = 2;
                            mappa[taxi_r][taxi_c+1].carattere = 'S';
                        }
                    }
                    if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                        if(mappa[taxi_r][taxi_c].tipo == 0){            //CELLA VUOTA    
                            mappa[taxi_r][taxi_c].tipo = 3;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }else if(mappa[taxi_r][taxi_c].tipo == 2){      //CELLA CON SORGENTE
                            mappa[taxi_r][taxi_c].tipo = 4;
                            mappa[taxi_r][taxi_c].carattere = '*';
                        }
                    }
                    mappa[taxi_r][taxi_c].numTaxiPresenti++;
                    *tempoImpiegato += mappa[taxi_r][taxi_c].tempAttravers.tv_nsec; 
                    if(*tempoImpiegato > SO_TIMEOUT){
                        mappa[taxi_r][taxi_c].numTaxiPresenti--;
                        if(mappa[taxi_r][taxi_c].numTaxiPresenti == 0){
                            if(mappa[taxi_r][taxi_c].tipo == 3){          //CELLA SOLO DI TAXI
                                mappa[taxi_r][taxi_c].tipo = 0;
                                mappa[taxi_r][taxi_c].carattere = '0';
                            }else if(mappa[taxi_r][taxi_c].tipo == 4){    //CELLA MISTA DI TAXI E SORGENTI
                                mappa[taxi_r][taxi_c].tipo = 2;
                                mappa[taxi_r][taxi_c].carattere = 'S';
                            }
                        }
                        printf("Richiesta non soddisfatta nei tempi previsti\n");
                        if(releaseSem(semid, 7) == -1){    //Incremento semaforo.
                            EXIT_ON_ERROR
                        }
                        return 0;
                    }

                    if(releaseSem(semid, 7) == -1){    //Incremento semaforo.
                        EXIT_ON_ERROR
                    } 
                    
                }
            }   //Fine else ostacolo
        }   //Fine while righe

        if(taxi_r == arrivo_r && taxi_c == arrivo_c){

            printf("Sono arrivato a destinazione.\n");
            //implementare capacità, semafori, ecc...
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

    int i,j;
    //inizializzazione valori dell'header
    SO_HOLES = 0; 
    SO_SOURCES = 0; 
    SO_TOP_CELLS = 0; 

    //definisco i valori random (ho messo un valore come massimo che si può togliere)
    //per avere un numero random compreso in un itervallo: rand() % (upper - lower + 1)) + lower; 
    srand(time(NULL)); 
    SO_CAP_MAX = rand() % 20; //num casuale tra 0 e RAND_MAX (as defined by the header file <stdlib.h>).
    SO_CAP_MIN = rand() %(SO_CAP_MAX+1); //ovviamente deve essere inferiore o uguale al max, metto il +1 includere la possibilità di averlo uguale
    SO_TIMENSEC_MAX = rand() % 1000; 
    SO_TIMENSEC_MIN = rand() %(SO_TIMENSEC_MAX+1);
    
    
    //creazione mappa inizializzata a 0:
    for(i=0; i<SO_HEIGHT; i++){
        for(j=0; j<SO_WIDTH; j++){
            map[i][j].tipo = 0;
            map[i][j].carattere = '0';
            map[i][j].cellaCoord[0] = i;
            map[i][j].cellaCoord[1] = j;
            //da implementare poi nel taximover:
            map[i][j].numTaxiPresenti = 0; 
            map[i][j].numVolteAttr = 0;
            map[i][j].tempAttravers.tv_sec = 1; 
            map[i][j].tempAttravers.tv_nsec = (rand() % (SO_TIMENSEC_MAX - SO_TIMENSEC_MIN +1)) + SO_TIMENSEC_MIN; //numero random tra l'intervallo
            map[i][j].capMax = (rand() % (SO_CAP_MAX - SO_CAP_MIN +1)) + SO_CAP_MIN;    //Numero massimo di taxi che la cella può contenere
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

    //Genero un numero random di buchi da creare nella mappa.
    numHoles = rand() %(SO_HEIGHT*SO_WIDTH-1);

    while(numHoles!=0){
        //Genero coordinate randomiche
        r = rand() %SO_HEIGHT;
        c = rand() %SO_WIDTH; 
        
        //Controllo che le coordinate generate rispettino i requisiti per generare un buco.
        if(controlloBuchi(A, r, c)){
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

    //Genero un numero random di buchi da creare nella mappa.
    numSources = rand() %(SO_HEIGHT*SO_WIDTH-SO_HOLES);
    
    while(numSources!=0){
        //Genero coordinate randomiche
        r = rand() %SO_HEIGHT;
        c = rand() %SO_WIDTH; 
      
        //Controllo che le sorgenti generate rispettino i requisiti (non devono sovrapporsi)
        if((A[r][c].tipo!= 1) && (A[r][c].tipo != 2)){ //"2" indica sulla mappa la posizione della cella sorgente
           A[r][c].tipo = 2;
           A[r][c].carattere = 'S';
           SO_SOURCES++; 
        }
    
    	numSources--;   	
    } 
}


void TaxiGenerator(struct cella A[SO_HEIGHT][SO_WIDTH], struct taxi *infoTaxi){

    int r, c;
    int flag = 1;
   
    while(flag){
        //Genero coordinate randomiche
        r = rand() %SO_HEIGHT;
        c = rand() %SO_WIDTH; 

        //controllo che ci sia spazio nella cella per il nuovo taxi
        if(A[r][c].numTaxiPresenti <= A[r][c].capMax){
            //Controllo che le sorgenti generate rispettino i requisiti (non devono sovrapporsi)
            if((A[r][c].tipo!= 1)){ //cella non bloccata!
                A[r][c].carattere = '*';
                A[r][c].numVolteAttr++;
                A[r][c].numTaxiPresenti++; 
                if((A[r][c].tipo == 0) || (A[r][c].tipo == 3) ){ 
                    A[r][c].tipo = 3; //è una cella vuota e diventa taxi
                }else if(A[r][c].tipo == 2){    
                    A[r][c].tipo = 4; //è una cella che è contiene sia un taxi che una sorgente
                }           
                flag = 0;	
            }
        }
              	
    }  
    ptMemCond->SO_TAXI++;
    infoTaxi->pid = (long)getpid();
    infoTaxi->percorso = 0;
    infoTaxi->viaggioPiuLungo = 0;
    infoTaxi->richiesteRacc = 0;
    infoTaxi->tempoImpiegato = 0;
    infoTaxi->coordTaxi[0] = r;
    infoTaxi->coordTaxi[1] = c;

}

//Metodo che mi dà delle coordinate a caso di arrivo
void getCellaArrivo(struct cella A[SO_HEIGHT][SO_WIDTH], struct queue *coda){ //passo la struttura per puntatore così non perdo dati

   int r,c;
   int flag = 1;

   while(flag){
       r = rand() %SO_HEIGHT;
       c = rand() %SO_WIDTH; 

       if((A[r][c].tipo!=1) && (coda->partenza[0]!=r && coda->partenza[1]!=c)){     //Controlliamo che le coordinate di arrivo siano diverse da un ostacolo e dalle coordinate di partenza!
           coda->arrivo[0] = r;
           coda->arrivo[1] = c;
           flag = 0;     
       }  
    }       
}


//====================================================================
//                      FUNZIONI PER SEMAFORI
//====================================================================

// Initialize semaphore to 1 (i.e., "available")
int initSemAvailable(int semId, int semNum) {
	union semun arg;
	arg.val = 1;

	return semctl(semId, semNum, SETVAL, arg);
}

// Initialize semaphore to 0 (i.e., "in use")
int initSemInUse(int semId, int semNum) {
	union semun arg;
	arg.val = 0;

	return semctl(semId, semNum, SETVAL, arg);
}

// Reserve semaphore - decrement it by 1
int reserveSem(int semId, int semNum) {
	struct sembuf sops;

	sops.sem_num = semNum;
	sops.sem_op = -1;
	sops.sem_flg = 0;

	return semop(semId, &sops, 1);
}

// Release semaphore - increment it by 1
int releaseSem(int semId, int semNum) {
	struct sembuf sops;
	
	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(semId, &sops, 1);
}

