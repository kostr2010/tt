CC = gcc
CFLAGS = -I. -Wall

all: main clear

main: tree.o main.o tree.h diff.h diff.o
	$(CC) -o main main.o tree.o diff.o

clear:
	rm -rf tree.o main.o diff.o