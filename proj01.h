/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.h						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/timeb.h>	// možná není an merlinu
#include <sys/time.h>

#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>

#define _POSIX_C_SOURCE 200809L
//#include <pthread.h>
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
#define ERR_PARAM 1
#define ERR_SHM 2
#define ERR_ELSE 3
#define EXIT_OK 0

/*
 * Struktura pro sdílenou pamět.
 */
typedef struct{
	int N;
	int M;
	int next;
	int now;
	int ticket;
} SharedMemory;

/*
 * Funkce pro ověření číselnosti parametrů.
 */
bool isNumber (char *argument);

/*
 * Funkce pro vytvoření semínka pro random generátor
 * Zdroj: http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
 */
unsigned int getSeed();

/*
 * Funkce pro uspání vlákna.
 */
void threadSleep(int random);

/*
 * Funkce pro zpracování parametrů.
 */
bool parseArguments(char* argv[], int argc);

/* 
 * Výstupní hodnotou této funkce je unikátní číslo lístku, který určuje pořadí
 * vstupu do kritické sekce. První získaný lístek má hodnotu 0, další 1, 2, atd.
 */
int getticket(void);

/* 
 * Vstup do kritické sekce, kde parametr aenter je číslo přiděleného lístku funkcí getticket(). 
 * Na počátku programu je vstup umožněn jen vláknu s lístkem 0. V kritické sekci může být v daném 
 * okamžiku maximálně jedno vlákno.
 */
void await(int aenter,int id);

/*
 * Výstup z kritické sekce, což umožní vstup jinému vláknu přes funkci await() s lístkem
 * o jedničku vyšším, než mělo vlákno kritickou sekci právě opouštějící.
 */
void advance(void);