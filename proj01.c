/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.c						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include "proj01.h"

pthread_mutex_t ks_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoření semaforu	
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoření semaforu

pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;	// Vytvoření podmínky


// Sdílená pamět
SharedMemory *sharedTicket;
int N;
int M;

/*
 * Funkce pro ověření číselnosti parametrů.
 */
int isNumber (char *argument)
{
    char *ptr;
    long arg = strtol(argument, &ptr,10);                   
    if (*ptr != '\0')
	{
		fprintf(stderr, "parseArguments() error\nUsage: proj01 [-N \"počet vláken\"] [-M \"počet průchodů KS\"]\n");
		exit(ERR_PARAM);		// Nesprávně zadané parametry		
	}
    return (int)arg;
}

/*
 * Funkce pro zpracování parametrů.
 */
bool parseArguments(char* argv[], int argc)
{
	int opt;
	bool isSetN = false;		// Nastavení příznaku zpracování povinného parametru
	bool isSetM = false;		// Nastavení příznaku zpracování povinného parametru
    while ((opt = getopt(argc, argv, "N:M:")) != -1) {		// Zpracování parametrů
        switch (opt) {
			case 'N': 
				N = isNumber(optarg);
				isSetN = true;	// Nastavení příznaku zpracování povinného parametru - zadán
				break;
			case 'M': 
				M = isNumber(optarg);
				isSetM = true;	// Nastavení příznaku zpracování povinného parametru - zadán
				break;
			default:
				fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"počet vláken\"] [-M \"počet průchodů KS\"]\n", argv[0]);
				exit(ERR_PARAM);		// Nesprávně zadané parametry
        }
    }
	
	if(optind < argc){				// Získání posledního parametru
		M = isNumber(argv[optind]); 
		isSetM = true;		// Nastavení příznaku zpracování povinného parametru - zadán
	}
	
	if(!isSetN || !isSetM || N < 0 || M < 0){
		fprintf(stderr, "parseArguments() error\nUsage: %s [-N \"počet vláken\"] [-M \"počet průchodů KS\"]\n", argv[0]);
		exit(ERR_PARAM);		// Nesprávně zadané parametry
	}
	
	return true;
}

/*
 * Funkce pro vytvoření semínka pro random generátor
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
 * Výstupní hodnotou této funkce je unikátní číslo lístku, který určuje pořadí
 * vstupu do kritické sekce. První získaný lístek má hodnotu 0, další 1, 2, atd.
 */
int getticket(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket->ticket++;
//	printf("Zvyšuji hodnotu lístku na: %d\n", sharedTicket->ticket);
	pthread_mutex_unlock( &ticket_mutex );	//unclock
	
	return sharedTicket->ticket;
}

/* 
 * Vstup do kritické sekce, kde parametr aenter je číslo přiděleného lístku funkcí getticket(). 
 * Na počátku programu je vstup umožněn jen vláknu s lístkem 0. V kritické sekci může být v daném 
 * okamžiku maximálně jedno vlákno.
 */
void await(int aenter, int id)	// TODO - odstranit int id po doladění
{
	pthread_mutex_lock( &ks_mutex );	// lock
//	printf("IN: lock.now = %d => aenter = %d (%d)\n",sharedTicket->now, aenter,id);
	while(sharedTicket->now != aenter)
		pthread_cond_wait( &condition_var, &ks_mutex );	
		// TODO - Ověřit funkčnost
}

/*
 * Výstup z kritické sekce, což umožní vstup jinému vláknu přes funkci await() s lístkem
 * o jedničku vyšším, než mělo vlákno kritickou sekci právě opouštějící.
 */
void advance(void)
{
	/* TODO - tady ta inkrementace musí být ATOMICKÁ*/
	pthread_mutex_lock( &ticket_mutex );	// lock
	sharedTicket->now++;
	sharedTicket->next++;
	pthread_mutex_unlock( &ticket_mutex );	// lock
	
//	printf("OUT: lock.now = %d => lock.next = %d\n",sharedTicket->now, sharedTicket->next);
	pthread_cond_broadcast( &condition_var );
	// pthread_cond_signal( &condition_var ); - povolí spuštění posledního vlákna, broadcast probudí všechny
	pthread_mutex_unlock( &ks_mutex );	//unclock
}

/*
 * Funkce pro obslužní rutinu každého vlákna.
 */
void *thread_function(void *i)
{
//	printf("Hodnota \"i\" ve funkci vlákna: %d\n", *((int *)i));
	int id = *((int *)i);
	free(i);
	int ticket;
   
	unsigned int seed = getSeed();
	
	while ((ticket = getticket()) < sharedTicket->M) { /* Přidělení lístku */
//		printf("Thread ID: %d (ticket: %d)\n", id,ticket);
		/* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
		await(ticket,id);		/* Vstup do KS */
		printf("Ticket: %d (%d)\n", ticket, id); /* fflush(stdout); */
		fflush(stdout);
		advance();              /* Výstup z KS */
		/* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
		threadSleep(rand_r(&seed) / (RAND_MAX/MAX_TIME));
	}	   
	
	return NULL;
}

int main(int argc, char* argv[]) 
{
	// Proměnné pro cykly
	int i;
	int j;
	
	// Zpracování argumentů
	parseArguments(argv, argc);
	
	// Vytvoření sdílené paměti
	int shm_id = shm_open(SHM_SPEC, O_CREAT | O_EXCL | O_RDWR, RIGHTS);
	
	if (shm_id < ZERO){
		fprintf(stderr, "shm_open() error\n");
		close(shm_id);
		exit(ERR_SHM);
	}
	// Místo pro strukturu
	if(ftruncate(shm_id, sizeof(SharedMemory))){
		fprintf(stderr, "ftruncate() error\n");
		shm_unlink(SHM_SPEC);
		close(shm_id);
		exit(ERR_SHM);
	}
	// Namapování sdílené paměti
	sharedTicket = (SharedMemory*)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
	//Inicializace sdílené paměti
	sharedTicket->ticket = -1;
	sharedTicket->now = 0;
	sharedTicket->next = 1;
	sharedTicket->N = N;
	sharedTicket->M = M;

	// Pole ID jednotlivých vláken
	pthread_t thread_id[sharedTicket->N];

	printf("Vytvářím vlákna...\n");
	// Tvorba vláken
	for(i=1; i <= sharedTicket->N; i++)
	{
		int *arg = (int*)malloc(sizeof(*arg));
		*arg = i;
		pthread_create(&thread_id[i], NULL, thread_function, arg);
	}

	// Ukončení vláken
	for(j=1; j <= sharedTicket->N; j++)
	{		
		pthread_join(thread_id[j], NULL);
	}		

	munmap(sharedTicket, sizeof(SharedMemory));
	shm_unlink(SHM_SPEC);
	close(shm_id);
	return (EXIT_SUCCESS);
}