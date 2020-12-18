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


#define EXIT_ON_ERROR if (errno) {fprintf(stderr,  \
              "%d: pid %ld; errno: %d (%s)\n",     \
              __LINE__,                            \
              (long) getpid(),                     \
              errno,                               \
              strerror(errno)); exit(EXIT_FAILURE);}



unsigned int SO_WIDTH = 5;
unsigned int SO_HEIGHT = 5;
//runtime:
unsigned int SO_HOLES = 0; //numero celle inaccessibili (<= W x H)
unsigned int SO_TIMESEC_MIN; //tempo minimo attraversamento cella (nanosec)
unsigned int SO_TIMESEC_MAX; //tempo massimo attraversamento cella (nanosec)
unsigned int SO_CAP_MIN; //num minimo taxi ospitati in una cella
unsigned int SO_CAP_MAX; //num massimo taxi ospitati in una cella
unsigned int SO_SOURCES; //num celle origine taxi (<= W x H - HOLES )
unsigned int SO_TIMEOUT; //tempo massimo di risposta del taxi
unsigned int SO_DURATION; //durata di simulazione
unsigned int SO_TOP_CELLS; //num celle più attraversate 
