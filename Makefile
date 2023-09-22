CXX = g++

OBJ = ./obj
BIN = ./bin
INC = ./inc
SRC = ./src

CXXFLAGS = -std=c++20 -I $(INC) -I /opt/local/include
LDFLAGS = -L /opt/local/lib -lraylib -lm -lpthread -lX11

all: dirs run

run: $(BIN)/main	
	$(BIN)/main

# driver
$(BIN)/main: $(OBJ)/main.o $(OBJ)/quad_tree.o $(OBJ)/particle.o
	$(CXX) $(LDFLAGS) $(OBJ)/main.o $(OBJ)/quad_tree.o $(OBJ)/particle.o -o $(BIN)/main

$(OBJ)/main.o: $(SRC)/main.cpp $(INC)/quad_tree.hpp $(INC)/particle.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC)/main.cpp -o $(OBJ)/main.o

$(OBJ)/particle.o: $(SRC)/particle.cpp $(INC)/particle.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC)/particle.cpp -o $(OBJ)/particle.o

$(OBJ)/quad_tree.o: $(SRC)/quad_tree.cpp $(INC)/quad_tree.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC)/quad_tree.cpp -o $(OBJ)/quad_tree.o

dirs:
	mkdir -p $(BIN)
	mkdir -p $(OBJ)

clean:
	rm -rf $(BIN) $(OBJ)