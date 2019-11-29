#include <SDL2/SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "Chip8.h"

// Will emulate chip8 given a ROM file
// TODO: add in an option for disassembler, maybe through a flag
int main(int argc, char** argv)
{
	// usage nagger
	if (argc != 2) 
	{
		printf("USAGE: chip8 [chip-8 ROM file]\n");
		exit(1);	
	}

	// open target ROM file
	FILE* f = fopen(argv[1], "r");
	if (!f)
	{
		printf("ERROR: Could not open \"%s\"\n", argv[1]);
		exit(1);
	}
	
	
	// determine file size
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	// create chip-8 and load ROM into it
	Chip8State* chip8 = InitChip8();
	fread(chip8->memory + 0x200, fsize, 1, f);
	fclose(f);

	// user interface setup
	SDL_Window* window;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Chip8Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);

	if (!window)
	{
		printf("ERROR: Failed to create user interface!\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	SDL_Texture* texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 32);

	// initializing chip8 display
	uint32_t pixels[64 * 32];
	int i;
	for (i = 0; i < 64 * 32; i++)
	{
		pixels[i] = 0xff000000;
	}


	int quit = 0;	
	while(!quit)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) // poll & handle all events before continuing
		{
			if (e.type == SDL_QUIT) {quit = 1;} // this only handles pressing 'x' on the window
		}

		printf("PC:%04x, I:%03x, V0:%02x, V1:%02x, INST:%02x%02x\n", chip8->PC, chip8->I, chip8->V[0], chip8->V[1], chip8->memory[chip8->PC], chip8->memory[chip8->PC + 1]);
		EmulateChip8Operation(chip8);
		SDL_Delay(1000);
		
		//SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		//SDL_RenderClear(render); // clear screen to background color
		
		// DO TEXTURE WRITING HERE

		//printf("attempting to copy display from chip8 to SDL texture");
		//uint8_t pixel;
		//for (pixel = 0; pixel < 64 * 32; pixel++)
		//{
		//	uint8_t target = chip8->display[pixel/8] >> (7 - (pixel % 8)) & 0x1;
		//	if (target) pixels[pixel] = 	0xffffffff;
		//	else pixels[pixel] = 		0xff000000;
		//}
		// END TEXTURE WRITING HERE
		
		//printf("attempting to update sdl texture");
		//SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
		//printf("attempting to copy texture to screen");
		//SDL_RenderCopy(render, texture, NULL, NULL);


		//printf("attempting to present current framebuffer");
		//SDL_RenderPresent(render);
	}
	
	// cleanup
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	DeleteChip8(chip8);
	exit(1);
}
