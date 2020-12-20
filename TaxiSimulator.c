#include "TaxiHeader.h"

//protipi funzioni:
void mapGenerator(int map[SO_HEIGHT][SO_WIDTH] ){
    int i,j; //riga e colonna
    //creazione matrice inizializzata a 0:
    for(i=0; i<SO_HEIGHT; i++){
        for(j=0; j<SO_WIDTH; j++)
            map[i][j] = 0;
    }
}

void printMap(int map[SO_HEIGHT][SO_WIDTH]){
    int i,j; //riga e colonna
    for(i=0; i<SO_HEIGHT; i++){
        for(j=0; j<SO_WIDTH; j++)
            printf("%d\t", map[i][j]);
            
    puts("\n");
    } 
}

int controllo(int A[SO_HEIGHT][SO_WIDTH], int i, int j){

    int R = SO_HEIGHT;
    int C = SO_WIDTH;
    
    if(i == 0 && j ==0){ //ALTO SX

		if((A[i][j] != 1) && (A[i][j+1] != 1) && (A[i+1][j] != 1) && (A[i+1][j+1] != 1)){
			return 1;
		
		}		
				
	}else if(i > 0 && j > 0 && i<(R-1) && j<(C-1)){	//CENTRALE esclusi gli estremi

		if((A[i][j]!=1) && (A[i-1][j-1]!=1) && (A[i-1][j]!=1) && (A[i-1][j+1]!=1) && (A[i][j-1]!=1) && (A[i][j+1]!=1) && (A[i+1][j-1]!=1) && (A[i+1][j]!=1) &&(A[i+1][j+1]!=1)){
			return 1;
					
		}

	}else if(i == 0 && j>0 && j<(C-1)){	//ALTO centro

		if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i][j+1] != 1) && (A[i+1][j-1] != 1) && (A[i+1][j] != 1)&& (A[i+1][j+1] != 1)){
			return 1;
					
		}	

	}else if(i == (R-1) && j>0 && j<(C-1)){	//BASSO centro

		if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i][j+1] != 1) && (A[i-1][j-1] != 1) && (A[i-1][j] != 1)&& (A[i-1][j+1] != 1)){
			return 1;
					
		}	

	}else if(i>0 && i<(R-1)  && j==0){	//SX centro

		if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i+1][j] != 1) && (A[i-1][j+1] != 1) && (A[i][j+1] != 1)&& (A[i+1][j+1] != 1)){
			return 1;
					
		}		

	}else if(i>0 && i<(R-1)  && j==(C-1)){	//DX centro

		if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i+1][j] != 1) && (A[i-1][j-1] != 1) && (A[i][j-1] != 1)&& (A[i+1][j-1] != 1)){
			return 1;
					
		}			
				

	}else if(i == 0 && j == (C-1)){	//ALTO DX

		if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i+1][j] != 1) && (A[i+1][j-1] != 1)){
			return 1;
					
		}

	}else if(i == (R-1) && j == 0){ //BASSO SX

		if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i][j+1] != 1) && (A[i-1][j+1] != 1)){
			return 1;
					
		}

	}else if(i == (R-1) && j == (C-1)){ //BASSO DX

		if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i][j-1] != 1) && (A[i-1][j-1] != 1)){
			return 1;
					
		}
	}
    
    return 0; 
            
}

void HolesGenerator(int A[SO_HEIGHT][SO_WIDTH]){

    int numHoles, r, c;

    srand(time(NULL));
    //Genero un numero random di buchi da creare nella mappa.
    numHoles = rand() %(SO_HEIGHT*SO_WIDTH-1);

    //printf("i buchi dovevano essere: %d\n", numHoles);

    while(numHoles!=0){
        //Genero coordinate randomiche
        c = rand() %SO_WIDTH; 
        r = rand() %SO_HEIGHT;

        //Controllo che le coordinate generate rispettino i requisiti per generare un buco.
        if(controllo(A, r, c)){
            //printf("riga: %d\n", r);
            //printf("colonna: %d\n", c);
            A[r][c] = 1;
            SO_HOLES++;     
        }
        else{
            EXIT_ON_ERROR
        }

        numHoles--;   
    }   

}

void SourcesGenerator(int A[SO_HEIGHT][SO_WIDTH]){

    int numSources, r, c;

    srand(time(NULL));
    //Genero un numero random di buchi da creare nella mappa.
    numSources = rand() %(SO_HEIGHT*SO_WIDTH-SO_HOLES);

    //printf("le sorgenti dovevano essere: %d\n", numSources);
    
    while(numSources!=0){
        //Genero coordinate randomiche
        c = rand() %SO_WIDTH; 
        r = rand() %SO_HEIGHT;
        //printf("riga: %d\n", r);
        //printf("colonna: %d\n", c);

        //Controllo che le sorgenti generate rispettino i requisiti (non devono sovrapporsi)
        if((A[r][c] != 1) && (A[r][c] != 2)){ //"2" indica sulla mappa la posizione della cella sorgente
            A[r][c] = 2;
            SO_SOURCES++;     
        }
        else{
        	EXIT_ON_ERROR
        }

    	numSources--;   
    	
    }  


}

void TaxiMover(int mappa[SO_HEIGHT][SO_WIDTH]){

    int taxi_r, taxi_c, arrivo_r, arrivo_c, flagR, flagC, finalflag;

    printf("Inserisci coordinate taxi: ");

    scanf("%d", &taxi_r);
    scanf("%d", &taxi_c);

    printf("Inserisci coordinate di arrivo: ");

    scanf("%d", &arrivo_r);
    scanf("%d", &arrivo_c);

    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
    printf("arrivo[%d][%d]\n", arrivo_r, arrivo_c);

    flagR = 1;
    flagC = 1;
    finalflag = 1;

    while(finalflag){   //Contenitore

        while(taxi_r<SO_HEIGHT && flagR){   //Righe

            while(taxi_c<SO_WIDTH && flagC){    //Colonne
                
                if((taxi_c < arrivo_c) && (mappa[taxi_r][taxi_c + 1] != 1)){
                    taxi_c++;
                    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                }else if((taxi_c > arrivo_c)  && (mappa[taxi_r][taxi_c - 1] != 1)){
                    taxi_c--;
                    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                }else if(taxi_c == arrivo_c){
                    printf("Ho raggiunto la stessa colonna\n");
                    flagC = 0;
                    break;
                }else{  //Ostacolo

                    if(taxi_r + 1 < SO_HEIGHT){
                        printf("Ostacolo, vado in basso\n");
                        taxi_r++;
                    }else if(taxi_r - 1 >= 0){
                        printf("Ostacolo, vado in alto\n");
                        taxi_r--;
                    }
                }

            }   //Fine while colonne
        
            if((taxi_r < arrivo_r)  && (mappa[taxi_r + 1][taxi_c] != 1)){
                    taxi_r++;
                    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                }else if((taxi_r > arrivo_r)  && (mappa[taxi_r - 1][taxi_c] != 1)){
                    taxi_r--;
                    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                }else if(taxi_r == arrivo_r){
                    printf("Ho raggiunto la stessa riga\n");
                    flagR = 0;
                    break;
                }else{  //Ostacolo
                   
                    if(taxi_c + 1 < SO_WIDTH){
                        printf("Ostacolo, vado a dx\n");
                        taxi_c++;
                    }else if(taxi_c - 1 >= 0){
                        printf("Ostacolo, vado a sx\n");
                        taxi_c--;
                    }

                }

        }   //Fine while righe

        if(taxi_r == arrivo_r && taxi_c == arrivo_c){
            printf("Arrivo\n");
            finalflag = 0;
            break;
        }else{
            printf("Continua\n");
            flagR = 1;
            flagC = 1;
        }

    }   //Fine while contenitore

    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);

}

//main:
int main(int argc, char *argv[]){

    int mappa[SO_HEIGHT][SO_WIDTH];
    int quantBuchi;
    int c, r;

    //Inizializzo la mappa
    mapGenerator(mappa);
    //stampo mappa inizializzata
    printMap(mappa);
    //Genero celle inaccesibili sulla mappa
    HolesGenerator(mappa);
    //Genero celle sorgenti
    SourcesGenerator(mappa);
    //stampo la mappa con i buchi e le sorgenti
    printMap(mappa);  
    printf("i buchi sono: %d\n", SO_HOLES);
    printf("le sorgenti sono: %d\n", SO_SOURCES);
    
    TaxiMover(mappa);

    
/*
SO_HOLES = 0 //verrà incrementato alla generazione di un buco
e sarà una coppia (1,1), un array che indica riga e colonna
funzione che verifica gli altri SO_holes */

}

