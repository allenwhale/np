CC=g++
INCLUDE=./include
SRC=./src
SOCKS4_INCLUDE=$(INCLUDE)/socks4
SOCKS4_SRC=$(SRC)/socks4
CGI_INCLUDE=$(INCLUDE)/cgi
CGI_SRC=$(SRC)/cgi
OBJ=./obj
BIN=./bin
SOCKS4_CFLAGS=-I. -I$(SOCKS4_INCLUDE) -O2 --std=c++11 -Wno-unused-result
CGI_CFLAGS=-I. -I$(CGI_INCLUDE) -O2 --std=c++11 -Wno-unused-result

all: $(BIN)/socks4.out $(BIN)/hw4.cgi
	@echo done

$(BIN)/socks4.out: $(OBJ)/socks4.o $(OBJ)/socks4_req.o $(OBJ)/socks4_firewall.o
	$(CC) $(SOCKS4_CFLAGS) -o $(BIN)/socks4.out $(OBJ)/socks4.o $(OBJ)/socks4_req.o $(OBJ)/socks4_firewall.o

$(OBJ)/socks4.o: $(SOCKS4_SRC)/socks4.cpp $(SOCKS4_INCLUDE)/socks4.h
	$(CC) $(SOCKS4_CFLAGS) -c -o $(OBJ)/socks4.o $(SOCKS4_SRC)/socks4.cpp

$(OBJ)/socks4_req.o: $(SOCKS4_SRC)/socks4_req.cpp $(SOCKS4_INCLUDE)/socks4_req.h
	$(CC) $(SOCKS4_CFLAGS) -c -o $(OBJ)/socks4_req.o $(SOCKS4_SRC)/socks4_req.cpp

$(OBJ)/socks4_firewall.o: $(SOCKS4_SRC)/socks4_firewall.cpp $(SOCKS4_INCLUDE)/socks4_firewall.h
	$(CC) $(SOCKS4_CFLAGS) -c -o $(OBJ)/socks4_firewall.o $(SOCKS4_SRC)/socks4_firewall.cpp

$(BIN)/hw4.cgi: $(OBJ)/client.o $(OBJ)/hw4.o
	$(CC) $(CGI_CFLAGS) -o $(BIN)/hw4.cgi $(OBJ)/client.o $(OBJ)/hw4.o

$(OBJ)/client.o: $(CGI_SRC)/client.cpp $(CGI_INCLUDE)/client.h
	$(CC) $(CGI_CFLAGS) -c -o $(OBJ)/client.o $(CGI_SRC)/client.cpp

$(OBJ)/hw4.o: $(CGI_SRC)/hw4.cpp $(CGI_INCLUDE)/hw4.h
	$(CC) $(CGI_CFLAGS) -c -o $(OBJ)/hw4.o $(CGI_SRC)/hw4.cpp

clean:
	rm -rf bin/*
	rm -rf obj/*
