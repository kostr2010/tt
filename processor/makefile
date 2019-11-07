CC = gcc
CFLAGS = -lm -I. -Wall

all: exec

clear:
	rm -rf stack.o executor.o

exec: stack.o executor.o stack.h cmds.h
	$(CC) -o execute executor.o stack.o -lm


