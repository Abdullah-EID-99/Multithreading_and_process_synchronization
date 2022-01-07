CC = gcc

all: lokanta

lokanta: lokanta.c
	$(CC) lokanta.c -o lokanta -l pthread

clean:
	rm -rf *o all
