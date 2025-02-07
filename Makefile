# Author: Ben Martens (1349551)

CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L$(BIN)
INC = include/
SRC = src/
BIN = bin/

all: test_main

test_main: main.o parser
	$(CC) $(CFLAGS) -o $(BIN)test_main main.o $(LDFLAGS) -lvcparser

main.o: $(SRC)main.c $(INC)VCParser.h $(INC)LinkedListAPI.h
	$(CC) -I$(INC) $(CFLAGS) -c $(SRC)main.c

parser: VCParser.o LinkedListAPI.o
	$(CC) -shared -o $(BIN)libvcparser.so VCParser.o LinkedListAPI.o

VCParser.o: $(SRC)VCParser.c $(INC)VCParser.h $(INC)LinkedListAPI.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)VCParser.c

LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) -I$(INC) $(CFLAGS) -c -fpic $(SRC)LinkedListAPI.c

clean:
	rm -rf $(BIN)test_main $(BIN)*.so *.o