CC = gcc -Wall

ash: ash_new.o ash_path.o ash_main.c
	$(CC) -g ash_main.c ash_new.o ash_path.o -o ash 

ash_new.o: ash_new.c ash_new.h
	$(CC) -g -c ash_new.c

ash_path.o: ash_path.c ash_path.h
	$(CC) -g -c ash_path.c

clean:
	rm -rf ash ash_new.o ash_path.o
