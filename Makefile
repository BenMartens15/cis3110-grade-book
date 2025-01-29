CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L$(BIN)
INC = include/
SRC = src/
BIN = bin/

all: test_main

test_main: main.o parser
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_main main.o -lvcparser

main.o: $(SRC)main.c $(INC)VCParser.h
	$(CC) -I$(INC) $(CFLAGS) -c $(SRC)main.c

parser: VCParser.o
	$(CC) -shared -o $(BIN)libvcparser.so VCParser.o

VCParser.o: $(SRC)VCParser.c $(INC)VCParser.h LinkedListAPI.o
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)VCParser.c

LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)LinkedListAPI.c

clean:
	rm -rf test_main $(BIN)*.so *.o