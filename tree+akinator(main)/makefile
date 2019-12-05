CC = gcc
CFLAGS = -I. -Wall

all: main clear

main: tree.o main.o tree.h
	$(CC) -o main main.o tree.o

clear:
	rm -rf tree.o main.o