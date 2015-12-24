CC=g++
INCLUDE=./include
SRC=./src
OBJ=./obj
BIN=./bin
CFLAGS=-I. -I$(INCLUDE) -O2 --std=c++11 -Wno-unused-result

all: $(BIN)/socks4.out
	@echo done

$(BIN)/socks4.out: $(OBJ)/socks4.o $(OBJ)/socks4_req.o
	$(CC) $(CFLGAS) -o $(BIN)/socks4.out $(OBJ)/socks4.o $(OBJ)/socks4_req.o

$(OBJ)/socks4.o: $(SRC)/socks4.cpp $(INCLUDE)/socks4.h
	$(CC) $(CFLAGS) -c -o $(OBJ)/socks4.o $(SRC)/socks4.cpp

$(OBJ)/socks4_req.o: $(SRC)/socks4_req.cpp $(INCLUDE)/socks4_req.h
	$(CC) $(CFLAGS) -c -o $(OBJ)/socks4_req.o $(SRC)/socks4_req.cpp
