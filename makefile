
COMPILER=gcc
FLAGS= -lncurses -o bin -Wall -Wextra

all: main

main: main.c
	$(COMPILER) $(FLAGS) -g3 -Werror -Wno-error=unused-variable -Wno-error=format-overflow main.c 

release:
	$(COMPILER) $(FLAGS) -O2 main.c 

clean:
	rm bin

