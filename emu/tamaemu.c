#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tamaemu.h"

int PAGECT=20;

void tamaDumpHw(M6502 *cpu) {
	char *intfdesc[]={"FP", "SPI", "SPU", "FOSC/8K", "FOSC/2K", "x", "x", "TM0", 
			"EX", "x", "TBL", "TBH", "x", "TM1", "x", "x"};
	Tamagotchi *t=(Tamagotchi *)cpu->User;
//	TamaHw *hw=&t->hw;
	int i;
	int ien=t->ioreg[0x70]+(t->ioreg[0x71]<<8);
	printf("Ints enabled: (0x%X)", ien);
	for (i=0; i<16; i++) {
		if (ien&(1<<i)) printf("%s ", intfdesc[i]);
	}
	printf("\n");

}

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

static char implemented[]={
//	0 1 2 3 4 5 6 7 8 9 A B C D E F
	1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //00
	0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0, //10
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //20
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //30
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //40
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //50
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //60
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //70
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //80
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //90
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //A0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //B0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //C0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //D0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //E0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //F0
	};

uint8_t ioRead(M6502 *cpu, register word addr) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	TamaHw *hw=&t->hw;
	if (addr==R_PBDATA) {
		return hw->portBdata;
	} else {
		if (!implemented[addr&0xff]) {
			if (!cpu->Trace) printf("Unimplemented ioRd 0x%04X\n", addr);
			cpu->Trace=1;
		}
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
	} else {
		if (!implemented[addr&0xff]) {
			printf("unimplemented ioWr 0x%04X 0x%02X\n", addr, val);
			cpu->Trace=1;
		}
	}
	t->ioreg[addr-0x3000]=val;
}


#define CLK2HZ 0
#define CLK8HZ 1
#define CLK4HZ 2
#define CLK16HZ 3
#define CLK128HZ 4
#define CLK512HZ 8
#define CLK256HZ 9
#define CLK1KHZ 10
#define CLK32KHZ 11
#define CLKROSC 12
#define CLKTBL 13
#define CLKTBH 14
#define CLKT0 15
#define CLKVDD 16
#define CLKEXTI 17
#define CLKCNT 18

#define REG(x) t->ioreg[(x)-0x3000]

void tamaHwTick(Tamagotchi *t) {
	//Okay, we're kinda assuming this is called at 8MHz. We'll scale the rest accordingly.
	TamaHw *hw=&t->hw;
	int clock[CLKCNT];
	const int ticks[CLKCNT]={8000000/2, 8000000/8, 8000000/4, 8000000/16, 8000000/128, 8000000/512, 
				8000000/1000, 8000000/32768, 8000000/128 /*TBD*/, 1, 1, 1, 1, 1, 1, 1, 1, 1}; 
	const int tblClocks[]={CLK2HZ, CLK8HZ, CLK4HZ, CLK16HZ};
	const int tbhClocks[]={CLK128HZ, CLK512HZ, CLK256HZ, CLK1KHZ};
	

	int x;
	int tm0tick, tm1tick;
	hw->ticks++; //next tick
	//Generate clock pulses for other counters
	for (x=0; x<CLKCNT; x++) clock[x]=(((hw->ticks%ticks[x])==0)?1:0);

	//Select correct tbl/tbh timebases
	if ((REG(R_TIMBASE)&0xC)==0x00) clock[CLKTBL]=clock[CLK2HZ];
	if ((REG(R_TIMBASE)&0xC)==0x04) clock[CLKTBL]=clock[CLK8HZ];
	if ((REG(R_TIMBASE)&0xC)==0x08) clock[CLKTBL]=clock[CLK4HZ];
	if ((REG(R_TIMBASE)&0xC)==0x0C) clock[CLKTBL]=clock[CLK16HZ];
	if ((REG(R_TIMBASE)&0x3)==0x00) clock[CLKTBH]=clock[CLK128HZ];
	if ((REG(R_TIMBASE)&0x3)==0x04) clock[CLKTBH]=clock[CLK512HZ];
	if ((REG(R_TIMBASE)&0x3)==0x08) clock[CLKTBH]=clock[CLK256HZ];
	if ((REG(R_TIMBASE)&0x3)==0x0C) clock[CLKTBH]=clock[CLK1KHZ];

	

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
	tama->hw.portAdata=0xff;
	tama->hw.portBdata=0xfe;
	tama->hw.portCdata=0xff;
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



