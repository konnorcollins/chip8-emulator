#ifndef CHIP8_H_
#define CHIP8_H_

#include <stdint.h>

typedef struct Chip8State
{
	// memory pointers
	uint8_t* memory;
	uint8_t* display;

	// keyboard
	uint8_t K[16];
	uint8_t K_prev[16];

	uint8_t V[16]; // 16 8-bit general purpose registers
	uint16_t I; // 16-bit address register
	uint16_t PC; // program counter

	uint8_t SP; // stack pointer
	uint8_t DT; // delay timer
	uint8_t ST; // sound timer

	// emulator-dependant stuff
	uint8_t waiting_for_key_press;
	
} Chip8State;

Chip8State* InitChip8(void);
void DeleteChip8(Chip8State* state);

void EmulateChip8Operation(Chip8State* state);

#endif
