#
# Shell
# CS 241 Fall 2015
#

CC = clang
INC = -I.
FLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -g -std=c99

all: wtsh

vector.o: vector.c vector.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

log.o: log.c log.h vector.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

wtsh: shell.o log.o vector.o
	$(CC) $^ -o $@

shell.o: shell.c vector.h log.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

.PHONY : clean
clean:
	-rm -r -f *.o wtsh
