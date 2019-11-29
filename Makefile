.DEFAULT_GOAL := chip8
CC=gcc
CFLAGS=-I. -Wall
LIBS=-lSDL2
DEPS=Chip8.h
OBJ=Chip8.o Chip8Emu.o


%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

chip8: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

disassembler: Chip8Disassembler.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *~ chip8 disassembler

