//====================================
    //              PROVA TAXI              
//====================================

    int taxi_r, taxi_c, arrivo_r, arrivo_c, i, j, flag, finalflag;

    printf("Inserisci coordinate taxi: ");

    scanf("%d", &taxi_r);
    scanf("%d", &taxi_c);

    printf("Inserisci coordinate di arrivo: ");

    scanf("%d", &arrivo_r);
    scanf("%d", &arrivo_c);

    printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
    printf("arrivo[%d][%d]\n", arrivo_r, arrivo_c);

    flag = 1;
    finalflag = 1;
    i = 0;
    j = 0;


    while(i<SO_HEIGHT && finalflag){

        while(j<SO_WIDTH && flag){

            if(taxi_c == arrivo_c){    
                printf("Sono sulla stessa colonna dell'arrivo!\n");              //Sono sulla stessa colonna del punto di arrivo
                printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                flag = 0;                                                        //Non è più necessario scorrere le colonne.
            }else if(mappa[i][j+1] != 1){
                
                taxi_c++;                                                        //Il taxi va avanti di una colonna.
                printf("Taxi avanti di una colonna\n");
                printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
            }

            j++;

        }

        if( taxi_r == arrivo_r){
            if(taxi_c == arrivo_c){
                printf("ARRIVO!!!\n");
                printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
                finalflag = 0;
                break;
            }
            printf("Sono sulla stessa riga dell'arrivo\n");
            printf("TAXI[%d][%d]\n", taxi_r, taxi_c);
            i++;
        }else if(mappa[i+1][j] != 1){
            
            taxi_r++;                           //Il taxi va avanti di una riga.
            printf("Taxi avanti di una riga\n");
            printf("TAXI[%d][%d]\n", taxi_r, taxi_c);

        }else if(taxi_c == arrivo_c && taxi_r == arrivo_r){
            printf("ARRIVO!!!\n");
            finalflag = 0;
        }

        i++;

    }
