assembler.c - program responsible for making binary file of .asm

assemble - executable of assembler.c must be called with 2 args: filename.asm and filename.bin

executor.c - program responsible for executing binary files, created by assembler

execute - executable of executor.c must be called with 1 arg: filename.bin

disassembler.c - program responsible for restoring .asm file from .bin file

disassemble - executable of disassembler.c must be called with 2 args: filename.bin and filename.asm

fact.asm - program that calculates factorial

victim.asm - my test sheet

makefile - compiles code to two executables: assemble & execute
