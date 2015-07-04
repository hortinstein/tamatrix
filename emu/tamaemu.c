#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tamaemu.h"

int PAGECT=20;

unsigned char **loadRoms() {
	char fname[128];
	unsigned char **roms;
	int i, l;
	FILE *f;
	roms=malloc(sizeof(char*)*PAGECT);
	for (i=0; i<PAGECT; i++) {
		sprintf(fname, "rom/p%d.bin", i);
		f=fopen(fname, "rb");
		if (f==NULL) {
			perror(fname);
			exit(1);
		}
		fseek(f, 16*1024, SEEK_SET);
		roms[i]=malloc(32*1024);
		l=fread(roms[i], 1, 32768, f);
		printf("%s - %d bytes\n", fname, l);
		printf("%x %x\n", roms[i][0x3ffc], roms[i][0x3ffd]);
		fclose(f);
	}
	return roms;
}

void freeRoms(unsigned char **roms) {
	int i;
	for (i=0; i<PAGECT; i++) {
		free(roms[i]);
	}
	free(roms);
}

uint8_t ioRead(M6502 *cpu, register word addr) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	TamaHw *hw=&t->hw;
	if (addr==0x3000) {
		return hw->bankSel;
	} else if (addr==0x3001) {
		return hw->clkCtl;
	} else {
		printf("Unimplemented ioRd 0x%04X\n", addr);
//		cpu->Trace=1;
		return t->ioreg[addr-0x3000];
	}
}

void ioWrite(M6502 *cpu, register word addr, register byte val) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	TamaHw *hw=&t->hw;
	if (addr==0x3000) {
		if (val>20) {
			printf("Unimplemented bank: 0x%02X\n", val);
		} else {
			printf("Bank switch %d\n", val);
			hw->bankSel=val;
		}
	} else if (addr==0x3001) {
		hw->clkCtl=val;
	} else if (addr==0x3002) {
		//32KHz ena
	} else if (addr==0x3004) {
		//wdt_clr
	} else if (addr==0x3011) {
		//port A dir
	} else if (addr>=0x3040 && addr<0x304A) {
		//LCD stuff
	} else {
		printf("unimplemented ioWr 0x%04X 0x%02X\n", addr, val);
//		cpu->Trace=1;
	}
	t->ioreg[addr-0x3000]=val;
}

void tamaHwTick(Tamagotchi *t) {

}

uint8_t tamaReadCb(M6502 *cpu, register word addr) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	uint8_t r;
	if (addr<0x600) {
		r=t->ram[addr];
	} else if (addr>=0x1000 &&  addr<0x1200) {
		r=t->dram[addr-0x1000];
	} else if (addr>=0x3000 &&  addr<0x4000) {
		r=ioRead(cpu, addr);
	} else if (addr>=0x4000 &&  addr<0xc000) {
		r=t->rom[t->hw.bankSel][addr-0x4000];
	} else if (addr>=0xc000) {
		r=t->rom[0][addr-0xc000];
	} else {
		printf("TamaEmu: invalid read: addr 0x%02X\n", addr);
		cpu->Trace=1;
	}
//	printf("Rd 0x%04X 0x%02X\n", addr, r);
	return r;
}

void tamaWriteCb(M6502 *cpu, register word addr, register byte val) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
//	printf("Wr 0x%04X 0x%02X\n", addr, val);
	if (addr<0x600) {
		t->ram[addr]=val;
	} else if (addr>=0x1000 &&  addr<0x1200) {
		t->dram[addr-0x1000]=val;
	} else if (addr>=0x3000 &&  addr<0x4000) {
		ioWrite(cpu, addr, val);
	} else {
		printf("TamaEmu: invalid write: addr 0x%04X val 0x%02X\n", addr, val);
		cpu->Trace=1;
	}
}

byte Patch6502(register byte Op, register M6502 *R) {
	return 0;
}

byte Loop6502(register M6502 *R) {
	return 0;
}

Tamagotchi *tamaInit(unsigned char **rom) {
	Tamagotchi *tama=malloc(sizeof(Tamagotchi));
	tama->cpu=malloc(sizeof(M6502));
	tama->rom=rom;
	tama->cpu->Rd6502=tamaReadCb;
	tama->cpu->Wr6502=tamaWriteCb;
	tama->cpu->User=(void*)tama;
	tama->hw.bankSel=0;
	Reset6502(tama->cpu);
	return tama;
}



void tamaRun(Tamagotchi *tama, int cycles) {
	int n;
	int i;
	while (cycles>0) {
		n=Exec6502(tama->cpu, 1);
		n=(1-n); //n = cycles ran
		for (i=0; i<n; i++) {
			tamaHwTick(tama);
		}
		cycles-=n;
	}
}



