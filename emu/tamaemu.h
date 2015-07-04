#ifndef TAMAEMU_H
#define TAMAEMU_H

#include <stdint.h>
#include "M6502/M6502.h"


typedef struct {
	M6502 *cpu;
	unsigned char **rom;
	char ram[1536];
	char dram[512];
	uint8_t bankSel;
} Tamagotchi;


unsigned char **loadRoms();
void freeRoms(unsigned char **roms);

Tamagotchi *tamaInit(unsigned char **rom);
void tamaRun(Tamagotchi *tama, int cycles);


#endif