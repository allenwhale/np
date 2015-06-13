INCLUDE = ./include
SRC = ./src
BIN = ./bin
CXX = g++
FLAGS = -std=c++11 -g -Wall -Wextra -I$(INCLUDE) -I.
all: main_server main_client 
	@if [ ! -d "$(BIN)" ]; then	\
		mkdir "$(BIN)"; 		\
	fi

run_server: main_server
	#valgrind 
	$(BIN)/main_server.out

main_server: server.o main_server.o
	$(CXX) $(FLAGS) 				\
		-o $(BIN)/main_server.out 	\
		$(BIN)/server.o				\
		$(BIN)/main_server.o

server.o: $(SRC)/server.cpp $(INCLUDE)/server.h
	$(CXX) $(FLAGS) -c -o $(BIN)/server.o $(SRC)/server.cpp

main_server.o: $(SRC)/main_server.cpp
	$(CXX) $(FLAGS) -c -o $(BIN)/main_server.o $(SRC)/main_server.cpp


run_client: main_client
	#valgrind 
	$(BIN)/main_client.out

main_client: client.o main_client.o
	$(CXX) $(FLAGS) 				\
		-o $(BIN)/main_client.out 	\
		$(BIN)/client.o				\
		$(BIN)/main_client.o

client.o: $(SRC)/client.cpp $(INCLUDE)/client.h
	$(CXX) $(FLAGS) -c -o $(BIN)/client.o $(SRC)/client.cpp

main_client.o: $(SRC)/main_client.cpp
	$(CXX) $(FLAGS) -c -o $(BIN)/main_client.o $(SRC)/main_client.cpp

clean:
	rm $(BIN)/*.o
