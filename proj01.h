/************************************************
*	 Projekt: 	Projekt do p�edm�tu POS		    * 
* 				Ticket Algorithm			    *
*	Auto�i:	Bc. Jakub Stejskal <xstejs24>	    *
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
 * Struktura pro sd�lenou pam�t.
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
 * Funkce pro ov��en� ��selnosti parametr�.
 */
int isNumber (char *argument);

/*
 * Funkce pro vytvo�en� sem�nka pro random gener�tor
 * Zdroj: http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
 */
unsigned int getSeed();

/*
 * Funkce pro usp�n� vl�kna.
 */
void threadSleep(int random);

/*
 * Funkce pro zpracov�n� parametr�.
 */
bool parseArguments(char* argv[], int argc);
//void parseArguments(char* argv[], int argc);

/* 
 * V�stupn� hodnotou t�to funkce je unik�tn� ��slo l�stku, kter� ur�uje po�ad�
 * vstupu do kritick� sekce. Prvn� z�skan� l�stek m� hodnotu 0, dal�� 1, 2, atd.
 */
int getticket(void);

/* 
 * Funkce pro z�sk�n� ID procesu
 */
int getID(void);

/* 
 * Vstup do kritick� sekce, kde parametr aenter je ��slo p�id�len�ho l�stku funkc� getticket(). 
 * Na po��tku programu je vstup umo�n�n jen vl�knu s l�stkem 0. V kritick� sekci m��e b�t v dan�m 
 * okam�iku maxim�ln� jedno vl�kno.
 */
void await(int aenter);

/*
 * V�stup z kritick� sekce, co� umo�n� vstup jin�mu vl�knu p�es funkci await() s l�stkem
 * o jedni�ku vy���m, ne� m�lo vl�kno kritickou sekci pr�v� opou�t�j�c�.
 */
void advance(void);
