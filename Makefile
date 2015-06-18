CC=cc
CFLAGS=-std=c99 -Wall -pedantic -ggdb
LDFLAGS=-lSDL2 -lSDL2_image

all: galines

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

evolver.o: evolver.c
	$(CC) $(CFLAGS) -c evolver.c

svg-writer.o: svg-writer.c
	$(CC) $(CFLAGS) -c svg-writer.c

galines: main.o evolver.o svg-writer.o
	$(CC) $(CFLAGS) -o galines main.o evolver.o svg-writer.o $(LDFLAGS)

