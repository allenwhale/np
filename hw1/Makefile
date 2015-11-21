BIN=./bin
OBJ=./obj
INCLUDE=./include
SRC=./src
CC=g++
FLAGS=-I. -I$(INCLUDE) -std=c++11 -O2 -Wno-unused-result
all: $(BIN)/server.out
	$(BIN)/server.out

$(BIN)/server.out: $(OBJ)/server.o $(OBJ)/function.o $(OBJ)/command.o $(OBJ)/shell.o
	$(CC) $(FLAGS) $(OBJ)/server.o $(OBJ)/function.o $(OBJ)/command.o $(OBJ)/shell.o -o $(BIN)/server.out

$(OBJ)/server.o: $(SRC)/server.cpp $(INCLUDE)/server.h
	$(CC) $(FLAGS) -c $(SRC)/server.cpp -o $(OBJ)/server.o

$(OBJ)/function.o: $(SRC)/function.cpp $(INCLUDE)/function.h
	$(CC) $(FLAGS) -c $(SRC)/function.cpp -o $(OBJ)/function.o

$(OBJ)/command.o: $(SRC)/command.cpp $(INCLUDE)/command.h
	$(CC) $(FLAGS) -c $(SRC)/command.cpp -o $(OBJ)/command.o

$(OBJ)/shell.o: $(SRC)/shell.cpp $(INCLUDE)/shell.h
	$(CC) $(FLAGS) -c $(SRC)/shell.cpp -o $(OBJ)/shell.o

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
