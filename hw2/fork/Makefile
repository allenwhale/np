BIN=./bin
OBJ=./obj
INCLUDE=./include
SRC=./src
CC=g++
FLAGS=-g -I. -I$(INCLUDE) -std=c++11 -O2 -Wno-unused-result
all: $(BIN)/server.out
	$(BIN)/server.out

$(BIN)/server.out: $(OBJ)/server.o $(OBJ)/function.o $(OBJ)/command.o $(OBJ)/shell.o $(OBJ)/client.o $(OBJ)/msgq.o $(OBJ)/sem.o
	$(CC) $(FLAGS) $(OBJ)/server.o $(OBJ)/function.o $(OBJ)/command.o $(OBJ)/shell.o $(OBJ)/client.o $(OBJ)/msgq.o $(OBJ)/sem.o -o $(BIN)/server.out

$(OBJ)/server.o: $(SRC)/server.cpp $(INCLUDE)/server.h
	$(CC) $(FLAGS) -c $(SRC)/server.cpp -o $(OBJ)/server.o

$(OBJ)/function.o: $(SRC)/function.cpp $(INCLUDE)/function.h
	$(CC) $(FLAGS) -c $(SRC)/function.cpp -o $(OBJ)/function.o

$(OBJ)/command.o: $(SRC)/command.cpp $(INCLUDE)/command.h
	$(CC) $(FLAGS) -c $(SRC)/command.cpp -o $(OBJ)/command.o

$(OBJ)/shell.o: $(SRC)/shell.cpp $(INCLUDE)/shell.h
	$(CC) $(FLAGS) -c $(SRC)/shell.cpp -o $(OBJ)/shell.o

$(OBJ)/client.o: $(SRC)/client.cpp $(INCLUDE)/client.h
	$(CC) $(FLAGS) -c $(SRC)/client.cpp -o $(OBJ)/client.o

$(OBJ)/msgq.o: $(SRC)/msgq.cpp $(INCLUDE)/msgq.h
	$(CC) $(FLAGS) -c $(SRC)/msgq.cpp -o $(OBJ)/msgq.o

$(OBJ)/sem.o: $(SRC)/sem.cpp $(INCLUDE)/sem.h
	$(CC) $(FLAGS) -c $(SRC)/sem.cpp -o $(OBJ)/sem.o

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
