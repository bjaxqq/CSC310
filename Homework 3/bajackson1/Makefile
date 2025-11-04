all: assign3

clean:
	rm -rf assign3 assign3.o

assign3: assign3.c assign3.h
	gcc -o assign3 assign3.c -Wall

debug: assign3.c assign3.h
	gcc -o assign3 -D DEBUG assign3.c -Wall
