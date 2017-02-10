/************************************************
*	 Projekt: 	Projekt do předmětu POS		    * 
* 				Ticket Algorithm			    *
*	Autoři:	Bc. Jakub Stejskal <xstejs24>	    *
*	Nazev souboru: ticket.c						*
*			Datum:  9. 2. 2017					*
*			Verze:	1.0		                    *
************************************************/

#include "ticket.h"

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


int main(int argc, char* argv[]) 
{
	Arguments args;
	if(parseArguments(&args, argv, argc))
	{
		printf("Hello world!\n");
		printf("Počet argumentů: %d\n",argc);
		printf("ARGV: %s\n",argv[1]);

		printf("N: %d\n",args.N);
		printf("M: %d\n",args.M);
		return 0;
	}
	else
		return 1;

	

	
	
} 