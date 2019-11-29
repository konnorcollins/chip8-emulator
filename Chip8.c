#include <stdint.h>
#include <stdlib.h>

#include "Chip8.h"

void Operation_8xy(Chip8State* state, uint8_t regx, uint8_t regy, uint8_t lownib);
void Operation_Ex(Chip8State* state, uint8_t regx, uint8_t lowbyte);
void Operation_Fx(Chip8State* state, uint8_t regx, uint8_t lowbyte);
void Operation_NotImplemented(Chip8State* state);

Chip8State* InitChip8(void)
{
	Chip8State* state = calloc(sizeof(Chip8State), 1); // using calloc since it initializes to 0's
	
	state->memory = calloc(1024 * 4, 1); // chip-8 has 4kb of memory available to it (0x000..0xfff)
	state->display = state->memory + 0xf00; // display reserves 0xf00 - 0xfff
	state->PC = 0x200; // memory below 0x200 is reserved
	state->SP = 0; // 0xea0 - 0xeff is reserved for call stack and other variables
	state->waiting_for_key_press = 0x0; // emulator-specific flag for 'wait for key press' instruction

	return state;
}

void DeleteChip8(Chip8State* state)
{
	free(state->memory);
	free(state);
}

void EmulateChip8Operation(Chip8State* state)
{
	uint8_t* op = &(state->memory[state->PC]); // fetch current instruction
	int nib = (*op & 0xf0) >> 4; // checking first nib of first byte instead of making an operation table
	switch (nib)
	{
		
		case 0x00: //
			{
				int subop = (*(op + 1) & 0xff);
				if (subop == 0xEE) // RETURN operation
				{
					// return PC up one in the stack
					state->PC = state->memory[0xea0 + state->SP];
					state->SP -= 2;
				}
				else if (subop == 0xE0) // clear display
				{
					uint16_t i;
					for (i = 0; i < 0x100; i++) // in other words, set 0xf00..0xfff to 0's
					{
						state->display[i] = 0x00;
					}
					state->PC += 2;
				}
				else // passes instruction to another chip that I haven't implemented
				{
					// TODO look into the chip the command is passed to 
					Operation_NotImplemented(state);
					state->PC +=2;
				}
			}
			break;

		case 0x01: // jump to location specified by given value
			{
				uint16_t firsthalf = *op & 0x0f;
				firsthalf  = firsthalf << 8;
				uint16_t target = firsthalf | (*(op + 1) & 0xff);
				state->PC = target;
			}
			break;

		case 0x02: // call subroutine at given addr
			{
				uint16_t target = (*op & 0x0f << 8) | (*(op + 1) & 0xff);
				state->PC += 2;
				state->SP += 2;
				state->memory[0xea0 + state->SP] = state->PC;
				state->PC = target;
				
				// advance program counter to next instruction
				// advance stack pointer
				// save current program counter to stack
				// change program counter to target address
			}
			break;

		case 0x03: // skip next instruction if Vx equals given value
			{
				uint8_t value = state->V[*op & 0x0f];
				if (value == *(op + 1))
				{
					state->PC += 2;
				}
				state->PC += 2;
			}
			break;

		case 0x04: // skip next instruction if Vx does NOT equal given value
			{
				uint8_t value = state->V[*op & 0x0f];
				if (value != *(op + 1))
				{
					state->PC += 2;
				}
				state->PC += 2;
			}
			break;

		case 0x05: // skip next instruction if Vx equals Vy
			{
				uint8_t valuex = state->V[*op & 0x0f];
				uint8_t valuey = state->V[*(op + 1) & 0xf0 >> 4];
				if (valuex == valuey)
				{
					state->PC += 2;
				}
				state->PC += 2;
			}
			break;

		case 0x06: // load given value into Vx
			{
				uint8_t reg = *op & 0x0f;
				uint8_t value = *(op + 1);
				state->V[reg] = value;
				state->PC += 2;
			}
			break;

		case 0x07: // add given value into target register's value
			{
				uint8_t reg = *op & 0x0f;
				uint8_t val = *(op + 1);
				state->V[reg] += val;
				state->PC += 2;
			}
			break;

		case 0x08: // performs an operation on two given registers depending on the last four bits
			{
				uint8_t regx = *op & 0x0f;
				uint8_t regy = *(op + 1) & 0xf0 >> 4;
				uint8_t lownib = *(op+1) & 0x0f;
				Operation_8xy(state, regx, regy, lownib);
				state->PC += 2; // all operations are arithmetic, so I put the PC advancement here
			}
			break;

		case 0x09: // skip next instruction if values in both given registers match
			{
				uint8_t a = state->V[*op & 0x0f];
				uint8_t b = state->V[(*(op+1) & 0xf0) >> 4];
				if (a == b)
				{
					state->PC += 2;
				}
				state->PC += 2;
			}
			break;

		case 0x0a: // set I register equal to value (address)
			{
				uint16_t value = *op & 0x0f;
				value = value << 8;
				value = value | (*(op + 1));
				state->I = value;
				state->PC += 2;
			}
			break;

		case 0x0b: // JP Vx, nnn (jump to location equal to sum of Vx and nnn)
			{
				uint16_t reg0val = state->V[0];
				uint16_t target = (*op & 0x0f << 8) | (*(op + 1));
				state->PC = (reg0val + target);
			}
			break;

		case 0x0c: // generates random byte, AND it with given value into target register
			{
				uint8_t reg = (*op & 0x0f);
				uint8_t givenval = (*(op + 1) & 0xff);
				uint8_t randomval = rand() & (0xff);
				state->V[reg] = givenval & randomval;
				state->PC += 2;
			}
			break;

		case 0x0d: // DRW Vx, Vy, nibble (draw function yoshi:NIGHTMARE)
			{
				// memory location of sprite to draw
				uint16_t target = state->I;

				// registers containing (x,y) coordinates
				uint8_t regx = *op & 0x0f;
				uint8_t regy = (*(op + 1) & 0xf0) >> 4;

				// target coordinates (x,y) on display
				uint8_t x = state->V[regx];
				uint8_t y = state->V[regy];

				// height of sprite (width is always 8)
				uint8_t height = *(op + 1) & 0x0f;
				uint8_t turned_off_a_bit_flag = 0x0; // set to 1 if a bit is flipped off on display

				uint8_t i; // y-coordinate
				for (i = 0; i < height; i++)
				{
					uint8_t pixels_to_write = state->memory[target + i]; // grab row of pixels for sprite at {I}
					uint8_t pixels_on_display = state->display[x + y * 8];	

					uint8_t bit;
					for (bit = 0; bit < 8; bit++)
					{
						// check to see if there was a 'collision' (flipping a bit on display off)
						if (!turned_off_a_bit_flag && (((pixels_to_write >> (7 - bit)) & 0x1) && ((pixels_on_display >> (7 - bit)) & 0x1)))
						{
							turned_off_a_bit_flag = 0x1;
						}
					}
					state->display[x + y * 8] = pixels_on_display ^ pixels_to_write; 
				}
				state->V[15] = turned_off_a_bit_flag;
				state->PC += 2;
			}
			break;

		case 0x0e: // operation depends on lower byte of opcode
			{
				uint8_t regx = *op & 0x0f;
				uint8_t lowbyte = *(op + 1) & 0xff;
				Operation_Ex(state, regx, lowbyte);
			}
			break;

		case 0x0f: // operation depends on lower byte of opcode
			{
				uint8_t regx = *op & 0x0f;
				uint8_t lowbyte = *(op + 1) & 0xff;
				Operation_Fx(state, regx, lowbyte);
			}
			break;

		default:
			{
				//printf("ERROR: could not decode \"%02x%02x\" !\n", (*op & 0xff), (*(op + 1) & 0xff));
				Operation_NotImplemented(state);
				state->PC += 2;
			}
			break;
	}
}

void Operation_8xy(Chip8State* state, uint8_t regx, uint8_t regy, uint8_t lownib)
{
	switch (lownib)
	{
		case 0x00: // LD Vx, Vy
			{
				state->V[regx] = state->V[regy];
			}
			break;
		case 0x01: // OR Vx, Vy
			{
				uint8_t comp = state->V[regx];
				state->V[regx] = comp | state->V[regy];
			}
			break;
		case 0x02: // AND Vx, Vy
			{
				uint8_t comp = state->V[regx];
				state->V[regx] = comp & state->V[regy];
			}
			break;
		case 0x03: // XOR Vx, Vy
			{
				uint8_t comp = state->V[regx];
				state->V[regx] = comp ^ state->V[regy];
			}
		case 0x04: // ADD Vx, Vy
			{
				uint8_t x = state->V[regx];
				uint8_t y = state->V[regy];
				if (x + y > 0xff)
				{
					state->V[15] = 0x1; // overflow sets CARRY flag
				}
				else
				{
					state->V[15] = 0x0; 
				}
				state->V[regx] = (x + y) % 0xff;
			}
			break;
		case 0x05: // SUB Vx, Vy
			{
				uint8_t x = state->V[regx];
				uint8_t y = state->V[regy];
				if (x > y)
				{
					state->V[15] = 0x1; // not borrowing sets NOT BORROW flag
				}
				else
				{
					state->V[15] = 0x0;
				}
				state->V[regx] = (x - y) % 0xff;
			}
			break;
		case 0x06: // SHR Vx {, Vy}
			{
				uint8_t x = state->V[regx];
				if (x & 0x1) // least significant bit is set to 1?
				{
					state->V[15] = 0x1; // sets LOST DATA flag?
				}
				else
				{
					state->V[15] = 0x0;
				}
				state->V[regx] = x >> 1;
			}
			break;
		case 0x07: // SUBN Vx, Vy
			{
				uint8_t x = state->V[regx];
				uint8_t y = state->V[regy];
				if (y > x)
				{
					state->V[15] = 0x1; //  sets NOT BORROW flag
				}
				else
				{
					state->V[15] = 0x0;
				}
				state->V[regx] = (y - x) & 0xff;
			}
			break;
		case 0x0e: // SHL Vx {, Vy}
			{
				uint8_t x = state->V[regx];
				if (x & 0x80) // most significant bit is set to 1?
				{
					state->V[15] = 0x1; //  sets LOST DATA flag?
				}
				else
				{
					state->V[15] = 0x0;
				}
				state->V[regx] = (x << 1) & 0xff;
			}
			break;

		default:
			{
				Operation_NotImplemented(state);
			}
			break;
	}
}

void Operation_Ex(Chip8State* state, uint8_t regx, uint8_t lowbyte)
{
	uint8_t key = state->K[state->V[regx]]; // for the purposes of testing, we'll assume pressed = 0x1, otherwise 0x0
	switch (lowbyte)
	{
		case 0x9e: // check for key being down, skip next instruction if it is
			{
				if (key)
				{
					state->PC += 2;
				}	
			}
			break;
		case 0xa1: // check for key being up, skip next instruction if it is
			{
				if (!key)
				{
					state->PC += 2;
				}
			}
			break;
		default:
			{
				Operation_NotImplemented(state);
			}
			break;
	}
	state->PC += 2;
}

void Operation_Fx(Chip8State* state, uint8_t regx, uint8_t lowbyte)
{
	switch (lowbyte)
	{
		case 0x07: // LD Vx, DT  (loads delay timer into Vx)
			{
				state->V[regx] = state->DT;
				state->PC += 2;
			}
			break;
			
		case 0x0a:
			{
				// add a wait flag to chip8state struct?
				// if not set and this is called, set flag
				//	then don't advance PC
				// if set, check to see if key was pressed
				// 	if no key was pressed, do nothing
				// 	if a key was pressed, advance PC
			
				if (!state->waiting_for_key_press) // lock emulator into waiting for a key press
				{
					state->waiting_for_key_press = 0x1;
					uint8_t kindex;
					for (kindex = 0; kindex < 16; kindex++)
					{
						state->K_prev[kindex] = state->K[kindex];
					}
				}
				else	// stuck at current PC until key press occurs
				{
					// check for key press
					uint8_t kindex;
					for (kindex = 0; kindex < 16; kindex++)
					{
						if (state->K_prev[kindex] != state->K[kindex])
						{
							state->V[regx] = kindex;
							state->waiting_for_key_press = 0x0;
							state->PC += 2;
							break;
						}
					}
				}

			}
			break;

		case 0x15: // LD DT, Vx   (loads delay timer with Vx)
			{
				state->DT = state->V[regx];
				state->PC += 2;
			}
			break;

		case 0x18: // LD ST, Vx  (loads sound timer with Vx)
			{
				state->ST = state->V[regx];
				state->PC += 2;
			}
			break;

		case 0x1e: // ADD I, Vx  (adds Vx to address register)
			{
				uint8_t x = state->V[regx];
				uint8_t overflow_flag = 0x0;
				if (state->I + x > 0x0fff)
					overflow_flag = 0x1;
				state->I = (state->I + x) & 0x0fff; // reduce result to 12 bits to fit address range
				state->V[15] = overflow_flag;
				state->PC += 2;
			}
			break;

		case 0x29: // LD F, Vx (loads character sprite into I)
			{

			}
			break;

		case 0x33: // LD B, Vx  (loads decimal value of Vx into {I}..{I+2})
			{
				uint8_t x = state->V[regx];
				uint8_t hundreds = (x / 100) % 10;
				uint8_t tens = (x / 10) % 10;
				uint8_t ones = x % 10;
				state->memory[state->I] = hundreds; // decimal hundred's
				state->memory[state->I + 1] = tens; // decimal ten's
				state->memory[state->I + 2] = ones; // decimal one's
				state->PC += 2;
			}
			break;

		case 0x55: // LD [I], Vx (store registers V0-Vx in {I}..{I+x})
			{
				uint8_t i;
				for (i = 0; i < regx + 1; i++)
				{
					state->memory[state->I+i] = state->V[i];
				}
				state->PC += 2;
			}
			break;

		case 0x65: // LD Vx, [I] (loads registers V0-Vx with values stored at {I}..{I+x})
			{
				uint8_t i;
				for (i = 0; i < regx + 1; i++)
				{
					state->V[i] = state->memory[state->I+i];
				}
				state->PC += 2;
			}
			break;

		default:
			{
				Operation_NotImplemented(state);
				state->PC += 2;
			}
			break;
	}
}

void Operation_NotImplemented(Chip8State* state)
{
	// TODO maybe make an error log or something?
	// you really shouldn't reach here unless
	// A: you're loading a program that uses instructions from the Super-Chip-48
	// B: you've somehow jumped the program counter into sprite space
}
