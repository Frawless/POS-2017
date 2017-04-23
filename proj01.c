/************************************************
*	 Projekt: 	Projekt do pøedmìtu POS		    * 
* 				Ticket Algorithm			    *
*	Autoøi:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: proj01.c						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include "proj01.h"

pthread_mutex_t ks_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoøení semaforu	
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoøení semaforu
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoøení semaforu

pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;	// Vytvoøení podmínky



// Sdílená pamìt
SharedMemory sharedTicket = {0,0,1,0,-1,0};
int N;
int M;

/*
 * Funkce pro ovìøení èíselnosti parametrù.
 */
int isNumber (char *argument)
{
    char *ptr;
    long arg = strtol(argument, &ptr,10);                   
    if (*ptr != '\0')
	{
		fprintf(stderr, "parseArguments() error\nUsage: proj01 [\"pocet vlaken\"] [\"pocet pruchodu KS\"]\n");
		exit(ERR_PARAM);		// Nesprávnì zadané parametry		
	}
    return (int)arg;
}

/*
 * Funkce pro zpracování parametrù.
 */
bool parseArguments(char* argv[], int argc)
{
	int opt;
	bool isSetN = false;		// Nastavení pøíznaku zpracování povinného parametru
	bool isSetM = false;		// Nastavení pøíznaku zpracování povinného parametru
    while ((opt = getopt(argc, argv, "N:M:")) != -1) {		// Zpracování parametrù
        switch (opt) {
			case 'N': 
				N = isNumber(optarg);
				isSetN = true;	// Nastavení pøíznaku zpracování povinného parametru - zadán
				break;
			case 'M': 
				M = isNumber(optarg);
				isSetM = true;	// Nastavení pøíznaku zpracování povinného parametru - zadán
				break;
			default:
				fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"pocet vlaken\"] [-M \"pocet pruchodu KS\"]\n", argv[0]);
				exit(ERR_PARAM);		// Nesprávnì zadané parametry
        }
    }
	
	if(optind < argc){				// Získání posledního parametru
		M = isNumber(argv[optind]); 
		isSetM = true;		// Nastavení pøíznaku zpracování povinného parametru - zadán
	}
	
	if(!isSetN || !isSetM || N < 0 || M < 0){
		fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"pocet vlaken\"] [-M \"pocet pruchodu KS\"]\n", argv[0]);
		exit(ERR_PARAM);		// Nesprávnì zadané parametry
	}
	
	return true;
}

/*
 * Funkce pro vytvoøení semínka pro random generátor
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
 * Funkce pro uspání vlákna.
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
 * Výstupní hodnotou této funkce je unikátní èíslo lístku, který urèuje poøadí
 * vstupu do kritické sekce. První získaný lístek má hodnotu 0, dal¹í 1, 2, atd.
 */
int getticket(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket.ticket++;
//	printf("Zvy¹uji hodnotu lístku na: %d\n", sharedTicket.ticket);
	pthread_mutex_unlock( &ticket_mutex );	//unclock
	
	return sharedTicket.ticket;
}

/* 
 * Funkce pro získání ID procesu
 */
int getID(void)
{
	pthread_mutex_lock( &id_mutex );	// lock
	sharedTicket.ID++;
	pthread_mutex_unlock( &id_mutex );	//unclock
	
	return sharedTicket.ID;
}

/* 
 * Vstup do kritické sekce, kde parametr aenter je èíslo pøidìleného lístku funkcí getticket(). 
 * Na poèátku programu je vstup umo¾nìn jen vláknu s lístkem 0. V kritické sekci mù¾e být v daném 
 * okam¾iku maximálnì jedno vlákno.
 */
void await(int aenter)
{
	pthread_mutex_lock( &ks_mutex );	// lock
//	printf("IN: lock.now = %d => aenter = %d (%d)\n",sharedTicket.now, aenter,id);
	while(sharedTicket.now != aenter)
		pthread_cond_wait( &condition_var, &ks_mutex );	
}

/*
 * Výstup z kritické sekce, co¾ umo¾ní vstup jinému vláknu pøes funkci await() s lístkem
 * o jednièku vy¹¹ím, ne¾ mìlo vlákno kritickou sekci právì opou¹tìjící.
 */
void advance(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket.now++;
	sharedTicket.next++;
	pthread_mutex_unlock( &ticket_mutex );	// lock
	
//	printf("OUT: lock.now = %d => lock.next = %d\n",sharedTicket.now, sharedTicket.next);
	pthread_cond_broadcast( &condition_var );
	// pthread_cond_signal( &condition_var ); - povolí spu¹tìní posledního vlákna, broadcast probudí v¹echny
	pthread_mutex_unlock( &ks_mutex );	//unclock
}

/*
 * Funkce pro obslu¾ní rutinu ka¾dého vlákna.
 */
void *thread_function()
{
//	printf("Hodnota \"i\" ve funkci vlákna: %d\n", *((int *)i));
	int id = getID();
	//free(i);
	int ticket;
   
	unsigned int seed = getSeed();
	
	while ((ticket = getticket()) < sharedTicket.M) { /* Pøidìlení lístku */
//		printf("Thread ID: %d (ticket: %d)\n", id,ticket);
		/* Náhodné èekání v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
		await(ticket);		/* Vstup do KS */
		printf("%d\t(%d)\n", ticket, id); /* fflush(stdout); */
		fflush(stdout);
		advance();              /* Výstup z KS */
		/* Náhodné èekání v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
	}	   
	
	return NULL;
}

int main(int argc, char* argv[]) 
{
	// Zpracování argumentù
	if(!(argc == 3)){
		fprintf(stderr, "parseArguments() error\nUsage: %s [\"pocet vlaken\"] [\"pocet pruchodu KS\"]\n",argv[0]);
		exit(ERR_PARAM);
	}

	N = isNumber(argv[1]);	// Pocet vlaken
	M = isNumber(argv[2]);	// Pocet pruchodu KS

	sharedTicket.N = N;
	sharedTicket.M = M;

	// Promìnné pro cykly
	int i;
	int j;
 
	pthread_attr_t attr;
	void *result; 
	int res; 
	/* vytvoøení implicitních atributù */ 
	if ((res = pthread_attr_init(&attr)) != 0) { 
		printf("pthread_attr_init() err %d\n", res); 
		return 1; 
	} 
	/* nastavení typu vlákna v atributech */ 
	if ((res = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE)) != 0) { 
		printf("pthread_attr_setdetachstate() err %d\n", res); 
		return 1; 
	} 
	// Zpracování argumentù
	//parseArguments(argv, argc);
	
	// Pole ID jednotlivých vláken
	pthread_t thread_id[sharedTicket.N];

	// Tvorba vláken
	for(i=0; i < sharedTicket.N; i++)
	{
		res = pthread_create(&thread_id[i], &attr, thread_function, NULL);
		if (res){ 
			printf("pthread_create() error %d\n", res);
			return 1; 
		}
	}

	// Ukonèení vláken
	for(j=0; j < sharedTicket.N; j++)
	{		
		//pthread_join(thread_id[j], NULL);
		/* èekání na dokonèení a pøevzetí stavu */ 
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
