all: main

main: main.cpp
	g++ main.cpp -lncurses -o bin -g

