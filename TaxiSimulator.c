#include "TaxiHeader.h"

//protipi funzioni:
void mapGenerator(int map[SO_WIDTH][SO_HEIGHT] ){
    int i,j; //riga e colonna
    //creazione matrice inizializzata a 0:
    for(i=0; i<SO_WIDTH; i++){
        for(j=0; j<SO_HEIGHT; j++)
            map[i][j] = 0;
    }
}

void printMap(int map[SO_WIDTH][SO_HEIGHT]){
    int i,j; //riga e colonna
    for(i=0; i<SO_WIDTH; i++){
        for(j=0; j<SO_HEIGHT; j++)
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


//main:
int main(int argc, char *argv[]){

    //matrice 8x8
    int mappa[SO_HEIGHT][SO_WIDTH];
    //gnenero la mappa
    mapGenerator(mappa);
    //stampo la mappa
    printMap(mappa);
    //funzione creazione SO_HOLES:
    srand(time(NULL));

    int quantBuchi = rand() %(SO_HEIGHT*SO_WIDTH-1);
    printf("i buchi dovevano essere: %d\n", quantBuchi);
    int count = 0;
    while(quantBuchi!=0){
        int c = rand() %SO_HEIGHT; 
        printf("riga: %d\n", c);
        int r = rand() %SO_WIDTH;
        printf("colonna: %d\n", r);
        if(controllo(mappa, r, c)){
            mappa[r][c] = 1;
            count++;     
        }
        else{
            EXIT_ON_ERROR
        }
    quantBuchi--;   
    }   
    printMap(mappa);  
    printf("i buchi sono: %d\n", count);
/*
SO_HOLES = 0 //verrà incrementato alla generazione di un buco
e sarà una coppia (1,1), un array che indica riga e colonna
funzione che verifica gli altri SO_holes */

}

