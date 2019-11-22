CC = gcc
CFLAGS = -I. -Wall

all: main clear

main: list.o main.o list.h
	$(CC) -o main main.o list.o

clear:
	rm -rf list.o main.o