all:	gekko.exe dozer.exe
	rm -rf *.o
Frame.o: 	Frame.c Frame.h
	gcc -Wall -Wextra -c Frame.c
Ibex.o:		Ibex.c Ibex.h	
	gcc -Wall -Wextra -c Ibex.c
Config.o:	Config.c Config.h
	gcc -Wall -Wextra -c Config.c
Share.o:
	gcc -Wall -Wextra -c Share.c
lista.o:	lista.c	lista.h
	gcc -Wall -Wextra -c lista.c
lista_dozer.o:	lista_dozer.c	lista_dozer.h
	gcc -Wall -Wextra -c lista_dozer.c	
lista_a_la_venta.o:	lista_a_la_venta.c	lista_a_la_venta.h
	gcc -Wall -Wextra -c lista_a_la_venta.c
		
	
gekko.o: 	gekko.c gekko.h
	gcc -Wall -Wextra -c gekko.c 
dozer.o: 	dozer.c dozer.h
	gcc -Wall -Wextra -c dozer.c
gekko.exe: 	gekko.o Frame.o Ibex.o Config.o lista_dozer.o lista_a_la_venta.o
	gcc -lpthread gekko.o Frame.o Ibex.o Config.o lista_dozer.o lista_a_la_venta.o -Wall -Wextra -o gekko.exe 
dozer.exe:	dozer.o	Frame.o Config.o lista.o Share.o
	gcc -lpthread dozer.o Frame.o Config.o lista.o Share.o -Wall -Wextra -o dozer.exe 
clean:
	rm -rf *.o gekko.exe


