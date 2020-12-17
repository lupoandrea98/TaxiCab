#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <sys/sem.h>
#include <time.h>


#define R 1
#define C 5

void printMap(int map[R][C]){
    int i,j; //riga e colonna
    for(i=0; i<R; i++){
        for(j=0; j<C; j++)
            printf("%d\t", map[i][j]);
            
    puts("\n");
    } 
    
}

void mapGenerator(int map[R][C] ){
    int i,j; //riga e colonna
    //int valore = 0;
    //creazione matrice inizializzata a 0:
    for(i=0; i<R; i++){
        for(j=0; j<C; j++)
            map[i][j] = 0;
    }
}

int main(){
   int A[R][C];
   int count = 0;

   mapGenerator(A);

  srand(time(NULL));
  int r = rand() %R; 
  int c = rand() %C;

  A[r][c] = 1; 
  printMap(A);
  puts("\n");

 for(int i=0; i<R; i++){
		for(int j=0; j<C; j++){

			if(i == 0 && j ==0){ //ALTO SX

				if((A[i][j] != 1) && (A[i][j+1] != 1) && (A[i+1][j] != 1) && (A[i+1][j+1] != 1)){
					A[i][j] = 1;
					count++;
				}
				
			}else if(i > 0 && j > 0 && i<(R-1) && j<(C-1)){	//CENTRALE esclusi gli estremi

				if((A[i][j]!=1) && (A[i-1][j-1]!=1) && (A[i-1][j]!=1) && (A[i-1][j+1]!=1) && (A[i][j-1]!=1) && (A[i][j+1]!=1) && (A[i+1][j-1]!=1) && (A[i+1][j]!=1) &&(A[i+1][j+1]!=1)){
					A[i][j] = 1;
					count++;
				}

			}else if(i == 0 && j>0 && j<(C-1)){	//ALTO centro

				if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i][j+1] != 1) && (A[i+1][j-1] != 1) && (A[i+1][j] != 1)&& (A[i+1][j+1] != 1)){
					A[i][j] = 1;
					count++;
				}	

			}else if(i == (R-1) && j>0 && j<(C-1)){	//BASSO centro

				if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i][j+1] != 1) && (A[i-1][j-1] != 1) && (A[i-1][j] != 1)&& (A[i-1][j+1] != 1)){
					A[i][j] = 1;
					count++;
				}	

			}else if(i>0 && i<(R-1)  && j==0){	//SX centro

				if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i+1][j] != 1) && (A[i-1][j+1] != 1) && (A[i][j+1] != 1)&& (A[i+1][j+1] != 1)){
					A[i][j] = 1;
					count++;
				}		

			}else if(i>0 && i<(R-1)  && j==(C-1)){	//DX centro

				if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i+1][j] != 1) && (A[i-1][j-1] != 1) && (A[i][j-1] != 1)&& (A[i+1][j-1] != 1)){
					A[i][j] = 1;
					count++;
				}			
				

			}else if(i == 0 && j == (C-1)){	//ALTO DX

				if((A[i][j] != 1) && (A[i][j-1] != 1) && (A[i+1][j] != 1) && (A[i+1][j-1] != 1)){
					A[i][j] = 1;
					count++;
				}

			}else if(i == (R-1) && j == 0){ //BASSO SX

				if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i][j+1] != 1) && (A[i-1][j+1] != 1)){
					A[i][j] = 1;
					count++;
				}

			}else if(i == (R-1) && j == (C-1)){ //BASSO DX

				if((A[i][j] != 1) && (A[i-1][j] != 1) && (A[i][j-1] != 1) && (A[i-1][j-1] != 1)){
					A[i][j] = 1;
					count++;
				}
	        }
        }
    }

printf("%d\n", count);
printMap(A);
}