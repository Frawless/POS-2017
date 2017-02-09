#################################################
#	Projekt: Projekt do předmětu POS	#
#		 Ticket Algorithm		#
#	 Autoři: Bc. Jakub Stejskal <xstejs24>	#
# Nazev souboru: Makefile			#
#	  Datum: 9. 2. 2017		    	#		
#	  Verze: 1.0				#
#################################################

CC = gcc
CFLAGS = -std=c90 -Wall -pedantic -ansi -W -Wextra
LOGIN = xstejs24
PROJ_NAME = ticket
FILES = ticket.o 
PACK = *.c *.h Makefile

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
default: $(FILES)
	$(CC) $(CFLAGS) -o $(PROJ_NAME) $^
	
pack: clean
	rm -f $(LOGIN).zip
	zip -r $(LOGIN).zip $(PACK)
	
run:
	./$(PROJ_NAME) > result
	
clean:
	rm -f *.o *.out $(PROJ_NAME)
