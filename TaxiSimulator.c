#include "TaxiHeader.h"

//protipi funzioni:
void mapGenerator(int map[SO_WIDTH][SO_HEIGHT] ){
    int i,j; //riga e colonna
    int valore = 0;
    //creazione matrice inizializzata a 0:
    for(i=0; i<SO_WIDTH; i++){
        for(j=0; j<SO_HEIGHT; j++)
            map[i][j] = valore++;
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

void holesGenerator(int map[SO_WIDTH][SO_HEIGHT]){
    int 

    //srand(time(NULL));
    //r = rand() %(WxH-1)? ovvero 8x8-1 = 63 xk va da 0 a 63

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
    holesGenerator(mappa);

/*
SO_HOLES = 0 //verrà incrementato alla generazione di un buco
e sarà una coppia (1,1), un array che indica riga e colonna
funzione che verifica gli altri SO_holes */

}