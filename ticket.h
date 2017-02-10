/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.h						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include <pthread.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>

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
void await(int aenter);

/*
 * Výstup z kritické sekce, což umožní vstup jinému vláknu přes funkci await() s lístkem
 * o jedničku vyšším, než mělo vlákno kritickou sekci právě opouštějící.
 */
void advance(void);