#################################################
#	Projekt: Projekt do předmětu POS	#
#		 Ticket Algorithm		#
#	 Autoři: Bc. Jakub Stejskal <xstejs24>	#
# Nazev souboru: GNUakefile			#
#	  Datum: 9. 2. 2017		    	#		
#	  Verze: 1.0				#
#################################################

CC = gcc 
CFLAGS = -std=gnu99 -Wall -pedantic -W -Wextra -pthread -D_SVID_SOURCE -D_GNU_SOURCE -D_BSD_SOURCE
LOGIN = xstejs24
PROJ_NAME = proj01
FILES = proj01.o 
PACK = *.c *.h Makefile

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
default: $(FILES)
	$(CC) $(CFLAGS) -o $(PROJ_NAME) $^ -lrt
	
pack: clean
	rm -f $(LOGIN).zip
	zip -r $(LOGIN).zip $(PACK)
	
run:
	./$(PROJ_NAME)
	
clean:
	rm -f *.o *.out $(PROJ_NAME)