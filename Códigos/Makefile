all: shellso

shellso: main.o shell.o
	gcc -o shellso main.o shell.o

shellso.o: main.c shell.c
	gcc -c main.c shell.c

clean:
	rm -f *.o *~ 
