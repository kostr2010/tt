CC = gcc
CFLAGS = -lm -I. -Wall

all: assemble disassemble execute clear

assemble: text.o assembler.o cmds.h text.h
	$(CC) -o assemble assembler.o text.o

disassemble: disassembler.o cmds.h 
	$(CC) -o disassemble disassembler.o

execute: stack.o executor.o stack.h cmds.h stack.h
	$(CC) -o execute executor.o stack.o -lm

clear:
	rm -rf stack.o executor.o text.o assembler.o disassembler.o
