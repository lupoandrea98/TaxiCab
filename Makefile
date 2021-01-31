#MAKEFILE
CFLAGS = -std=c89 -pedantic     # flag per la normale compilazione

# flag per debugging, alcune parti del codice potrebbero richiedere la
# macro DEBUG
# CFLAGS = -std=c89 -pedantic -O0 -g -DDEBUG

OBJ1  = Master.c   
OBJ2  = Taxi.c   
OBJ3  = Source.c   
OBJ4  = TaxiSon.c  

# target di compilazione finale
TARGET1 =  Master     
TARGET2 =  Taxi
TARGET3 =  Source
TARGET4 =  TaxiSon

$(TARGET1): $(OBJ1)
	gcc $(CFLAGS) $(OBJ1) -o $(TARGET1)

$(TARGET2): $(OBJ2)
	gcc $(CFLAGS) $(OBJ2) -o $(TARGET2)
	gcc $(CFLAGS) $(OBJ4) -o $(TARGET4)
	
$(TARGET3): $(OBJ3)
	gcc $(CFLAGS) $(OBJ3) -o $(TARGET3)


# per la compilazione di tutti i programmi insieme
all: $(TARGET1) $(TARGET2) $(TARGET3)   

# per rimuovere tutti i file sorgenti ed eseguibili
clear:	
	rm -f *.o $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)  *~

# non lo useremo ma lo lasciamo giusto per burocrazia. Necessita dell'eseguibile
run:	$(TARGET1)	
	./$(TARGET1)
