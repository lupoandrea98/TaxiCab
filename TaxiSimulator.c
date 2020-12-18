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

    printMap(mappa);  
    printf("i buchi sono: %d\n", SO_HOLES);
    
/*
SO_HOLES = 0 //verrà incrementato alla generazione di un buco
e sarà una coppia (1,1), un array che indica riga e colonna
funzione che verifica gli altri SO_holes */

}

