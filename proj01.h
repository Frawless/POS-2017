/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.h						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#define _POSIX_C_SOURCE 199500L
#include <pthread.h>
//#include <limits.h>
//#ifdef _POSIX_THREADS
//	#include <pthread.h>
//#else
//	#error "POSIX threads are not available"
//#endif

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /* XPG 4.2 - needed for WCOREDUMP() */

#define MAX_TIME 500


#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/timeb.h>	// možná není an merlinu
#include <sys/time.h>

/*
 * Struktura pro vstupní parametry
 */
typedef struct{
	int N;
	int M;
	bool isSetN;
	bool isSetM;
} Arguments;

/*
 * Struktura pro pořadí.
 */
typedef struct{
	int next;
	int now;
} TLOCK;

/*
 * Funkce pro ověření číselnosti parametrů.
 */
bool isNumber (char *argument)
{
    char *ptr;
    strtod(argument, &ptr);                   
    if (*ptr == '\0')                          
        return false;
    return true;
}

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
bool parseArguments(Arguments *args, char* argv[], int argc);

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