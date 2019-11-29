#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void DisassembleChip8p(uint8_t* codebuffer, int pc)
{
	// each operation code is 2 bytes
	// first half (nibble) of first byte determines the operation
	uint8_t* code = &codebuffer[pc]; 
	uint8_t nib = (code[0] >> 4);
	printf("%04x %02x %02x ", pc, code[0], code[1]);
	
	switch(nib)
	{

		case 0x00: // multiple ops
			{
				if (code[1] == 0xE0) // Clear display
				{
					printf("CLS");
					break;
				}

				if (code[1] == 0xEE) // return from subroutine
				{
					printf("RET");
					break;
				}

				

				break;
			}	
		case 0x01: // jump to given address
			{
				printf("JP %01x%02x", (code[0] & 0x0f), code[1]);
				break;
			}
		
		case 0x02: // call subroutine at given address
			{
				printf("CALL %01x%02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x03: // compares value in given register to given value, skips next instruction if they match
			{
				printf("SE V%u, %02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x04: // compares value in given register to given value, skips next instruction if they don't match
			{
				printf("SNE V%u, %02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x05: // compares values in two given registers, skips next instruction if they match
			{
				printf("SE V%u, V%u", (code[0] & 0x0f), (code[1] & 0xf0) >> 4);
				break;
			}

		case 0x06: // load a value into a register
			{
				uint8_t reg = code[0] & 0x0f; // determine target register from remaining 4 bits of first byte
				printf("LD V%u, %02x", reg, code[1]);
				break;
			}

		case 0x07: // add given register's value with given value, store value in that register
			{
				uint8_t reg = code[0] & 0x0f;
				printf("ADD V%u, %02x", reg, code[1]);
				break;
			}

		case 0x08: // performs math op on two given registers
			{
				uint8_t regtarget = code[0] & 0x0f;
				uint8_t regsrc = (code[1] & 0xf0) >> 4;

				uint8_t mathop = code[1] & 0x0f;
				if (mathop == 0x00) // set value in one register equal to value in other register
				{
					printf("LD V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x01) // bitwise OR
				{
					printf("OR V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x02) // bitwise AND
				{
					printf("AND V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x03) // bitwise XOR
				{
					printf("XOR V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x04) // addition
				{
					printf("ADD V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x05) // subtraction
				{
					printf("SUB V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x06) // bitshift right
				{
					printf("SHR V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x07) // subtraction 2 (it's different somehow)
				{
					printf("SUBN V%u, V%u", regtarget, regsrc);
				}
				if (mathop == 0x0E) // bitshift left
				{
					printf("SHL, V%u, V%u", regtarget, regsrc);	
				}

				break;
			}

		case 0x09: // compares values in two given registers, if they don't match then skip the next instruction
			{
				uint8_t rega = code[0] & 0x0f;
				uint8_t regb = (code[1] & 0xf0) >> 4;
				printf("SNE V%u, V%u", rega, regb);
				break;
			}

		case 0x0a: // load an value (presumably an address) into the address register 
			{
				printf("LD I, %01x%02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x0b: // set program counter to address given plus the vaule in register 0
			{
				printf("JP V0, %01x%02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x0c: // generates a random number (0..255), AND's it with the given value, and stores result in target register
			{
				printf("RND V%u, %02x", (code[0] & 0x0f), code[1]);
				break;
			}

		case 0x0d: // draws n-byte sprite at address stored in I at position x,y, x and y are values in two given registers
			{
				uint8_t regx = code[0] & 0x0f;
				uint8_t regy = (code[1] & 0xf0) >> 4;
				uint8_t size = code[1] & 0x0f;
				printf("DRW V%u, V%u, %01x", regx , regy, size);
				break;
			}

		case 0x0e: // skip next instruction based on key specified in given register
			{
				uint8_t reg = code[0] & 0x0f;
				if (code[1] == 0x9e) // skip next if pressed
				{
					printf("SKP %u", reg);
				}
				if (code[1] == 0xa1) // skip next if not pressed
				{
					printf("SKNP %u", reg);
				}

				break;
			}

		case 0x0f: // misc ops
			{
				uint8_t reg = code[0] & 0x0f;

				if (code[1] == 0x07) // load delay timer value into register
				{
					printf("LD V%u, DT", reg);
				}
				if (code[1] == 0x0a) // wait for key press, store value of key into register
				{
					printf("LD V%u, K", reg);
				}
				if (code[1] == 0x15) // set delay timer equal to value in register
				{
					printf("LD DT, V%u", reg);
				}
				if (code[1] == 0x18) // set sound timer equal to value in register
				{
					printf("LD ST, V%u", reg);
				}
				if (code[1] == 0x1e) // add values in I and register, store result in I
				{
					printf("ADD I, V%u", reg);
				}
				if (code[1] == 0x29) // LD F, Vx
				{
					printf("LD F, V%u", reg);
				}
				if (code[1] == 0x33) // LD B, Vx
				{
					printf("LD B, V%u", reg);
				}
				if (code[1] == 0x55) // store contents of register 0 thru given register at addr stored in I
				{
					printf("LD [I], V%u", reg); 
				}
				if (code[1] == 0x65) // load contents of addr I in register 0 through given register
				{
					printf("LD V%u, [I]", reg);
				}

				break;
			}


		default:
			{
				printf("gb");
				break;
			}
	}
}

int main(int argc, char** argv)
{
	if (argc != 2) // improper usage
	{
		printf("USAGE: chip8 [chip-8 ROM]\n");
		exit(1);
	}

	FILE* f = fopen(argv[1], "rb"); // open chip-8 rom file
	if (!f)
	{
		printf("error: Couldn't open %s\n", argv[1]);
		exit(1);
	}

	// determine file size
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET); // reset cursor

	// read ROM into buffer at 0x200
	// this is because 0x200 is normally reserved for the interpreter
	unsigned char* buffer = malloc(fsize + 0x200);
	fread(buffer+0x200, fsize, 1, f);
	fclose(f);

	int pc = 0x200; // start program counter after reserved memory section
	while (pc < (fsize+0x200))
	{
		DisassembleChip8p(buffer, pc);
		pc += 2; // every opcode is 2 bytes
		printf("\n");
	}

	free(buffer);

	return 0;
}
