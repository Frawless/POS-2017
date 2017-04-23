/************************************************
*	 Projekt: 	Projekt do p�edm�tu POS		    * 
* 				Ticket Algorithm			    *
*	Auto�i:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: proj01.c						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include "proj01.h"

pthread_mutex_t ks_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvo�en� semaforu	
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvo�en� semaforu
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvo�en� semaforu

pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;	// Vytvo�en� podm�nky



// Sd�len� pam�t
SharedMemory sharedTicket = {0,0,1,0,-1,0};
int N;
int M;

/*
 * Funkce pro ov��en� ��selnosti parametr�.
 */
int isNumber (char *argument)
{
    char *ptr;
    long arg = strtol(argument, &ptr,10);                   
    if (*ptr != '\0')
	{
		fprintf(stderr, "parseArguments() error\nUsage: proj01 [\"pocet vlaken\"] [\"pocet pruchodu KS\"]\n");
		exit(ERR_PARAM);		// Nespr�vn� zadan� parametry		
	}
    return (int)arg;
}

/*
 * Funkce pro zpracov�n� parametr�.
 */
bool parseArguments(char* argv[], int argc)
{
	int opt;
	bool isSetN = false;		// Nastaven� p��znaku zpracov�n� povinn�ho parametru
	bool isSetM = false;		// Nastaven� p��znaku zpracov�n� povinn�ho parametru
    while ((opt = getopt(argc, argv, "N:M:")) != -1) {		// Zpracov�n� parametr�
        switch (opt) {
			case 'N': 
				N = isNumber(optarg);
				isSetN = true;	// Nastaven� p��znaku zpracov�n� povinn�ho parametru - zad�n
				break;
			case 'M': 
				M = isNumber(optarg);
				isSetM = true;	// Nastaven� p��znaku zpracov�n� povinn�ho parametru - zad�n
				break;
			default:
				fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"pocet vlaken\"] [-M \"pocet pruchodu KS\"]\n", argv[0]);
				exit(ERR_PARAM);		// Nespr�vn� zadan� parametry
        }
    }
	
	if(optind < argc){				// Z�sk�n� posledn�ho parametru
		M = isNumber(argv[optind]); 
		isSetM = true;		// Nastaven� p��znaku zpracov�n� povinn�ho parametru - zad�n
	}
	
	if(!isSetN || !isSetM || N < 0 || M < 0){
		fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"pocet vlaken\"] [-M \"pocet pruchodu KS\"]\n", argv[0]);
		exit(ERR_PARAM);		// Nespr�vn� zadan� parametry
	}
	
	return true;
}

/*
 * Funkce pro vytvo�en� sem�nka pro random gener�tor
 * Zdroj: http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
 */
unsigned int getSeed()
{
	unsigned int seed;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	struct timeval tv;
	gettimeofday(&tv,NULL);
	
	seed = tm.tm_hour*3600*1000*1000+tm.tm_min*60*1000*1000+tm.tm_sec*1000*1000+tv.tv_usec;
//	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return seed;
}

/*
 * Funkce pro usp�n� vl�kna.
 */
void threadSleep(int random)
{
   struct timespec tim, tim2;
   tim.tv_sec = 0;
   tim.tv_nsec = random*1000*1000;

   if(nanosleep(&tim , &tim2) < 0 )   
   {
      printf("Nano sleep system call failed \n");
   }

//   printf("Nano sleep successfull \n");

}


/* 
 * V�stupn� hodnotou t�to funkce je unik�tn� ��slo l�stku, kter� ur�uje po�ad�
 * vstupu do kritick� sekce. Prvn� z�skan� l�stek m� hodnotu 0, dal�� 1, 2, atd.
 */
int getticket(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket.ticket++;
//	printf("Zvy�uji hodnotu l�stku na: %d\n", sharedTicket.ticket);
	pthread_mutex_unlock( &ticket_mutex );	//unclock
	
	return sharedTicket.ticket;
}

/* 
 * Funkce pro z�sk�n� ID procesu
 */
int getID(void)
{
	pthread_mutex_lock( &id_mutex );	// lock
	sharedTicket.ID++;
	pthread_mutex_unlock( &id_mutex );	//unclock
	
	return sharedTicket.ID;
}

/* 
 * Vstup do kritick� sekce, kde parametr aenter je ��slo p�id�len�ho l�stku funkc� getticket(). 
 * Na po��tku programu je vstup umo�n�n jen vl�knu s l�stkem 0. V kritick� sekci m��e b�t v dan�m 
 * okam�iku maxim�ln� jedno vl�kno.
 */
void await(int aenter)
{
	pthread_mutex_lock( &ks_mutex );	// lock
//	printf("IN: lock.now = %d => aenter = %d (%d)\n",sharedTicket.now, aenter,id);
	while(sharedTicket.now != aenter)
		pthread_cond_wait( &condition_var, &ks_mutex );	
}

/*
 * V�stup z kritick� sekce, co� umo�n� vstup jin�mu vl�knu p�es funkci await() s l�stkem
 * o jedni�ku vy���m, ne� m�lo vl�kno kritickou sekci pr�v� opou�t�j�c�.
 */
void advance(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket.now++;
	sharedTicket.next++;
	pthread_mutex_unlock( &ticket_mutex );	// lock
	
//	printf("OUT: lock.now = %d => lock.next = %d\n",sharedTicket.now, sharedTicket.next);
	pthread_cond_broadcast( &condition_var );
	// pthread_cond_signal( &condition_var ); - povol� spu�t�n� posledn�ho vl�kna, broadcast probud� v�echny
	pthread_mutex_unlock( &ks_mutex );	//unclock
}

/*
 * Funkce pro obslu�n� rutinu ka�d�ho vl�kna.
 */
void *thread_function()
{
//	printf("Hodnota \"i\" ve funkci vl�kna: %d\n", *((int *)i));
	int id = getID();
	//free(i);
	int ticket;
   
	unsigned int seed = getSeed();
	
	while ((ticket = getticket()) < sharedTicket.M) { /* P�id�len� l�stku */
//		printf("Thread ID: %d (ticket: %d)\n", id,ticket);
		/* N�hodn� �ek�n� v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
		await(ticket);		/* Vstup do KS */
		printf("%d\t(%d)\n", ticket, id); /* fflush(stdout); */
		fflush(stdout);
		advance();              /* V�stup z KS */
		/* N�hodn� �ek�n� v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
	}	   
	
	return NULL;
}

int main(int argc, char* argv[]) 
{
	// Zpracov�n� argument�
	if(!(argc == 3)){
		fprintf(stderr, "parseArguments() error\nUsage: %s [\"pocet vlaken\"] [\"pocet pruchodu KS\"]\n",argv[0]);
		exit(ERR_PARAM);
	}

	N = isNumber(argv[1]);	// Pocet vlaken
	M = isNumber(argv[2]);	// Pocet pruchodu KS

	sharedTicket.N = N;
	sharedTicket.M = M;

	// Prom�nn� pro cykly
	int i;
	int j;
 
	pthread_attr_t attr;
	void *result; 
	int res; 
	/* vytvo�en� implicitn�ch atribut� */ 
	if ((res = pthread_attr_init(&attr)) != 0) { 
		printf("pthread_attr_init() err %d\n", res); 
		return 1; 
	} 
	/* nastaven� typu vl�kna v atributech */ 
	if ((res = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE)) != 0) { 
		printf("pthread_attr_setdetachstate() err %d\n", res); 
		return 1; 
	} 
	// Zpracov�n� argument�
	//parseArguments(argv, argc);
	
	// Pole ID jednotliv�ch vl�ken
	pthread_t thread_id[sharedTicket.N];

	// Tvorba vl�ken
	for(i=0; i < sharedTicket.N; i++)
	{
		res = pthread_create(&thread_id[i], &attr, thread_function, NULL);
		if (res){ 
			printf("pthread_create() error %d\n", res);
			return 1; 
		}
	}

	// Ukon�en� vl�ken
	for(j=0; j < sharedTicket.N; j++)
	{		
		//pthread_join(thread_id[j], NULL);
		/* �ek�n� na dokon�en� a p�evzet� stavu */ 
		if ((res = pthread_join(thread_id[j], &result)) != 0) { 
			printf("pthread_attr_init() err %d\n",res); 
			return 1; 
		} 
	}		

	//munmap(sharedTicket, sizeof(SharedMemory));
	//shm_unlink(SHM_SPEC);
	//close(shm_id);
	return (EXIT_SUCCESS);
}
