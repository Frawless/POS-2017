/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.c						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include "proj01.h"

/*
 * Funkce pro zpracování parametrů.
 */
bool parseArguments(Arguments *args, char* argv[], int argc)
{
	int opt;
	args->isSetN = false;		// Nastavení příznaku zpracování povinného parametru
	args->isSetM = false;		// Nastavení příznaku zpracování povinného parametru
    while ((opt = getopt(argc, argv, "N:M:")) != -1) {		// Zpracování parametrů
        switch (opt) {
			case 'N': 
				args->N = atoi(optarg); 
				args->isSetN = true;	// Nastavení příznaku zpracování povinného parametru - zadán
				break;
			case 'M': 
				args->M = atoi(optarg); 
				args->isSetM = true;	// Nastavení příznaku zpracování povinného parametru - zadán
				break;
			default:
				fprintf(stderr, "Usage: %s [-N \"počet vláken\"] [-M \"počet průchodů KS\"]\n", argv[0]);
				return false;		// Nesprávně zadané parametry
        }
    }
	
	if(optind < argc){				// Získání posledního parametru
		args->M = atoi(argv[optind]); 
		args->isSetM = true;		// Nastavení příznaku zpracování povinného parametru - zadán
	}
	
	if(!args->isSetM || !args->isSetM){
		printf("Špatně zadané argumenty!\nUsage: %s [-N \"počet vláken\"] [-M \"počet průchodů KS\"]\n",argv[0]);	
		return false; // Nesprávně zadané parametry
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
	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return seed;
}

/*
 * Funkce pro uspání vlákna.
 */
void threadSleep(int random)
{
   struct timespec tim, tim2;
   tim.tv_sec = 0;
   tim.tv_nsec = random*1000;

   if(nanosleep(&tim , &tim2) < 0 )   
   {
      printf("Nano sleep system call failed \n");
   }

   printf("Nano sleep successfull \n");

}



int counter = 0;
int ticketG = 0;
pthread_mutex_t ks_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoření semaforu	
pthread_mutex_t gen_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoření semaforu	
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;	// vytvoření semaforu

int getticket(void)
{
	pthread_mutex_lock( &ticket_mutex );	// lock
	printf("Zvyšuji hodnotu lístku na: ");
	ticketG++;
	printf("%d\n",ticketG);
	pthread_mutex_unlock( &ticket_mutex );	//unclock
	
	return ticketG;
	
}


void *thread_function(void *dummyPtr)
{
   printf("Thread number %ld\n", pthread_self());
   pthread_mutex_lock( &gen_mutex );
   counter++;
   pthread_mutex_unlock( &gen_mutex );
}

int main(int argc, char* argv[]) 
{
	Arguments args;
	if(parseArguments(&args, argv, argc))
	{
		int i;
		int j;
		pthread_t thread_id[args.N];

//		pthread_mutex_lock( &ks_mutex );	// lock
//		pthread_mutex_unlock( &ks_mutex );	//unclock
		
		printf("Vytvářím vlákna...\n");
		for(i=1; i <= args.N; i++)
		{
			pthread_create( &thread_id[i], NULL, thread_function, NULL );
			
		}
		
		for(j=1; j <= args.N; j++)
		{
			printf("ID vlákna ze struktury: %ld\n",thread_id[j]);
		}		

		for(j=1; j <= args.N; j++)
		{		
			pthread_join( thread_id[j], NULL);
		}
		
		unsigned int seed = getSeed();
		threadSleep(rand_r(&seed) / (RAND_MAX/500));
		
		int ticket;
		while ((ticket = getticket()) < args.M) { /* Přidělení lístku */
			/* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
			//await(ticket);              /* Vstup do KS */
			pthread_mutex_lock( &ks_mutex );	// lock
			printf("Výpis v KS: %d\t(%ld)\n", ticket, pthread_self()); /* fflush(stdout); */
			pthread_mutex_unlock( &ks_mutex );	//unclock
			//advance();              /* Výstup z KS */
			/* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
			//return 1;
		}	
		
struct timeval tv;
gettimeofday(&tv,NULL);
printf("Test: %d\n",tv.tv_sec);
printf("Test: %d\n",tv.tv_usec);
		return 0;
	}
	else
		return 1;

}
