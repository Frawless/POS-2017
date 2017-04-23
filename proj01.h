/************************************************
*	 Projekt: 	Projekt do pøedmìtu POS		    * 
* 				Ticket Algorithm			    *
*	Autoøi:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: proj01.h						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#if _POSIX_VERSION >= 200112L
	#include <pthread.h>
#else
	#error "POSIX threads are not available"
#endif

#define MAX_TIME 500
#define SHM_SPEC "/xstejs24_sharedMemory"
#define RIGHTS 0644
#define ZERO 0
#define ERR_PARAM -1
#define ERR_SHM 2
#define ERR_ELSE 3
#define EXIT_OK 0

/*
 * Struktura pro sdílenou pamìt.
 */
typedef struct{
	int N;
	int M;
	int next;
	int now;
	int ticket;
	int ID;
} SharedMemory;

/*
 * Funkce pro ovìøení èíselnosti parametrù.
 */
int isNumber (char *argument);

/*
 * Funkce pro vytvoøení semínka pro random generátor
 * Zdroj: http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
 */
unsigned int getSeed();

/*
 * Funkce pro uspání vlákna.
 */
void threadSleep(int random);

/*
 * Funkce pro zpracování parametrù.
 */
bool parseArguments(char* argv[], int argc);
//void parseArguments(char* argv[], int argc);

/* 
 * Výstupní hodnotou této funkce je unikátní èíslo lístku, který urèuje poøadí
 * vstupu do kritické sekce. První získaný lístek má hodnotu 0, dal¹í 1, 2, atd.
 */
int getticket(void);

/* 
 * Funkce pro získání ID procesu
 */
int getID(void);

/* 
 * Vstup do kritické sekce, kde parametr aenter je èíslo pøidìleného lístku funkcí getticket(). 
 * Na poèátku programu je vstup umo¾nìn jen vláknu s lístkem 0. V kritické sekci mù¾e být v daném 
 * okam¾iku maximálnì jedno vlákno.
 */
void await(int aenter);

/*
 * Výstup z kritické sekce, co¾ umo¾ní vstup jinému vláknu pøes funkci await() s lístkem
 * o jednièku vy¹¹ím, ne¾ mìlo vlákno kritickou sekci právì opou¹tìjící.
 */
void advance(void);
