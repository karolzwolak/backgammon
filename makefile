
COMPILER=gcc
FLAGS= -lncurses -o bin -Wall -Wextra

all: main

main: main.c
	$(COMPILER) $(FLAGS) -g3 -Werror -Wno-error=unused-variable -Wno-error=format-overflow -Wno-error=unused-parameter main.c 

release:
	$(COMPILER) $(FLAGS) -O2 main.c 

run: main
	./bin

clean:
	rm bin

