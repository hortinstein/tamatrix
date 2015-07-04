#ifndef TAMAEMU_H
#define TAMAEMU_H

#include <stdint.h>
#include "M6502/M6502.h"

typedef struct {
	uint8_t strobe;
	uint8_t dir;
	uint8_t config;
	uint8_t data;
} TamaGpio;

typedef struct {
	uint8_t bankSel;
	uint8_t clkCtl;
	TamaGpio gpioa;
	TamaGpio gpiob;
} TamaHw;


typedef struct {
	M6502 *cpu;
	unsigned char **rom;
	char ram[1536];
	char dram[512];
	char ioreg[255];
	TamaHw hw;
} Tamagotchi;


unsigned char **loadRoms();
void freeRoms(unsigned char **roms);

Tamagotchi *tamaInit(unsigned char **rom);
void tamaRun(Tamagotchi *tama, int cycles);


#endif