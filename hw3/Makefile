BIN=./bin
OBJ=./obj
INCLUDE=./include
SRC=./src
CC=g++
CGI_SRC=$(SRC)/cgi
CGI_INCLUDE=$(INCLUDE)/cgi
CGI_FLAGS=-g -I. -I$(CGI_INCLUDE) -std=c++11 -O2 -Wno-unused-result
HTTP_SRC=$(SRC)/http
HTTP_INCLUDE=$(INCLUDE)/http
HTTP_FLAGS=-g -I. -I$(HTTP_INCLUDE) -std=c++11 -O2 -Wno-unused-result

all: $(BIN)/hw3.cgi $(BIN)/http.out
	@echo done

$(BIN)/hw3.cgi: $(OBJ)/hw3.o $(OBJ)/client.o
	$(CC) $(CGI_FLAGS) $(OBJ)/hw3.o $(OBJ)/client.o -o $(BIN)/hw3.cgi 

$(OBJ)/hw3.o: $(CGI_SRC)/hw3.cpp $(CGI_INCLUDE)/hw3.h
	$(CC) $(CGI_FLAGS) -c $(CGI_SRC)/hw3.cpp -o $(OBJ)/hw3.o

$(OBJ)/client.o: $(CGI_SRC)/client.cpp $(CGI_INCLUDE)/client.h
	$(CC) $(CGI_FLAGS) -c $(CGI_SRC)/client.cpp -o $(OBJ)/client.o

$(BIN)/http.out: $(OBJ)/http.o $(OBJ)/server.o
	$(CC) $(HTTP_FLAGS) $(OBJ)/http.o $(OBJ)/server.o -o $(BIN)/http.out

$(OBJ)/http.o: $(HTTP_SRC)/http.cpp $(HTTP_INCLUDE)/http.h
	$(CC) $(HTTP_FLAGS) -c $(HTTP_SRC)/http.cpp -o $(OBJ)/http.o

$(OBJ)/server.o: $(HTTP_SRC)/server.cpp $(HTTP_INCLUDE)/server.h
	$(CC) $(HTTP_FLAGS) -c $(HTTP_SRC)/server.cpp -o $(OBJ)/server.o

clean:
	rm -rf $(BIN)/*.cgi $(BIN)/*.out
	rm -rf $(OBJ)/*
