#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tamaemu.h"

int PAGECT=20;

#define REG(x) t->ioreg[(x)-0x3000]


void tamaDumpHw(M6502 *cpu) {
	char *intfdesc[]={"FP", "SPI", "SPU", "FOSC/8K", "FOSC/2K", "x", "x", "TM0", 
			"EX", "x", "TBL", "TBH", "x", "TM1", "x", "x"};
	char *nmidesc[]={"LV", "TM1", "x", "x", "x", "x", "x", "NMIEN"};
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	TamaHw *hw=&t->hw;
	int i;
	int ien=t->ioreg[0x70]+(t->ioreg[0x71]<<8);
	printf("Ints enabled: (0x%X)", ien);
	for (i=0; i<16; i++) {
		if (ien&(1<<i)) printf("%s ", intfdesc[i]);
	}
	printf("\nInt flags:");
	for (i=0; i<16; i++) {
		if (hw->iflags&(1<<i)) printf("%s ", intfdesc[i]);
	}
	printf("\nNMI ena:");
	for (i=0; i<8; i++) {
		if (REG(R_NMICTL)&(1<<i)) printf("%s ", nmidesc[i]);
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
	1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0, //30
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //40
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //50
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //60
	1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0, //70
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
	} else if (addr==R_INTCTLLO) {
		return hw->iflags&0xff;
	} else if (addr==R_INTCTLMI) {
		return hw->iflags>>8;
	} else if (addr==R_NMICTL) {
		return hw->nmiflags;
	} else {
		if (!implemented[addr&0xff]) {
//			if (!cpu->Trace) printf("Unimplemented ioRd 0x%04X\n", addr);
//			cpu->Trace=1;
		}
		return t->ioreg[addr-0x3000];
	}
}

void ioWrite(M6502 *cpu, register word addr, register byte val) {
	Tamagotchi *t=(Tamagotchi *)cpu->User;
	TamaHw *hw=&t->hw;
	if (addr==R_BANK) {
		if (val>20) {
			printf("Unimplemented bank: 0x%02X\n", val);
		} else {
//			printf("Bank switch %d\n", val);
			hw->bankSel=val;
		}
	} else if (addr==R_INTCLRLO) {
		int msk=0xffff^(val);
		hw->iflags&=msk;
	} else if (addr==R_INTCLRMI) {
		int msk=0xffff^(val<<8);
		hw->iflags&=msk;
	} else if (addr==R_IFFPCLR) {
		hw->iflags&=~(1<<0);
	} else if (addr==R_IF8KCLR) {
		hw->iflags&=~(1<<3);
	} else if (addr==R_IF2KCLR) {
		hw->iflags&=~(1<<4);
	} else if (addr==R_IFTM0CLR) {
		hw->iflags&=~(1<<7);
	} else if (addr==R_IFTBLCLR) {
		hw->iflags&=~(1<<10);
	} else if (addr==R_IFTBHCLR) {
		hw->iflags&=~(1<<11);
	} else if (addr==R_IFTM1CLR) {
		hw->iflags&=~(1<<13);
	} else if (addr==R_LCDSEG) {
		t->lcd.sizex=(val+1)*8;
	} else if (addr==R_LCDCOM) {
		t->lcd.sizey=(val+1);
	} else {
		if (!implemented[addr&0xff]) {
//			printf("unimplemented ioWr 0x%04X 0x%02X\n", addr, val);
//			cpu->Trace=1;
		}
	}
	t->ioreg[addr-0x3000]=val;
}


#define CLK2HZ 0
#define CLK4HZ 1
#define CLK8HZ 2
#define CLK16HZ 3
#define CLK32HZ 4
#define CLK64HZ 5
#define CLK128HZ 6
#define CLK512HZ 7
#define CLK256HZ 8
#define CLK1KHZ 9
#define CLK32KHZ 10
#define CLKROSC 11
#define CLKTBL 12
#define CLKTBH 13
#define CLKT0 14
#define CLKVSS 15
#define CLKVDD 16
#define CLKEXTI 17
#define CLKD2K 18
#define CLKD8K 19
#define CLKCNT 20


void tamaHwTick(Tamagotchi *t) {
	//Okay, we're kinda assuming this is called at 8MHz. We'll scale the rest accordingly.
	TamaHw *hw=&t->hw;
	M6502 *R=t->cpu;
	int clock[CLKCNT];
	const int ticks[CLKCNT]={8000000/2, 8000000/4, 8000000/8, 8000000/16, 8000000/32, 8000000/64, 
				8000000/128, 8000000/512, 8000000/256, 8000000/1000, 8000000/32768, 
				8000000/128 /*TBD*/, 1, 1, 1, 1, 1, 1, 2048, 8192}; 
	const int tblClocks[]={CLK2HZ, CLK8HZ, CLK4HZ, CLK16HZ};
	const int tbhClocks[]={CLK128HZ, CLK512HZ, CLK256HZ, CLK1KHZ};
	const int tm0ClocksA[]={CLKVSS, CLKROSC, CLK32KHZ, CLKEXTI /*actually ECLK*/, CLKVDD};
	const int tm0ClocksB[]={CLKVDD, CLKTBL, CLKTBH, CLKEXTI, CLK2HZ, CLK8HZ, CLK32HZ, CLK64HZ};
	const int tm1Clocks[]={CLKVSS, CLKROSC, CLK32KHZ, CLKT0};
	int x;
	int ien=0;
	int tm0tick=0, tm1tick=0;
	int t0ovf=0, t1ovf=0;
	int nmiTrigger=0;
	hw->ticks++; //next tick
	//Generate clock pulses for other counters
	for (x=0; x<CLKCNT; x++) clock[x]=(((hw->ticks%ticks[x])==0)?1:0);
	clock[CLKVSS]=0; clock[CLKVDD]=0;

	//Select correct tbl/tbh timebases
	clock[CLKTBL]=clock[tblClocks[((REG(R_TIMBASE)&0xC)>>2)]];
	clock[CLKTBH]=clock[tbhClocks[((REG(R_TIMBASE)&0x3))]];

	//We don't do the AND of timer0 yet... 
	tm0tick=clock[tm0ClocksB[(REG(R_TIMCTL)>>2)&7]];
	if (tm0tick) {
		REG(R_TM0LO)++;
		if (REG(R_TM0LO)==0) {
			REG(R_TM0HI)++;
			if (REG(R_TM0HI)==0) {
				t0ovf=1;
			}
		}
	}
	clock[CLKT0]=t0ovf;

	tm1tick=clock[tm1Clocks[(REG(R_TIMCTL)&3)]];
	if (tm1tick) {
		REG(R_TM1LO)++;
		if (REG(R_TM1LO)==0) {
			REG(R_TM1HI)++;
			if (REG(R_TM1HI)==0) {
				t1ovf=1;
			}
		}
	}
	
	//Handle clock interrupts
	if (t1ovf) hw->iflags|=(1<<13);
	if (t1ovf) nmiTrigger|=(1<<1);
	if (clock[CLKTBH]) hw->iflags|=(1<<11);
	if (clock[CLKTBL]) hw->iflags|=(1<<10);
	if (t0ovf) hw->iflags|=(1<<7);
	if (clock[CLKD2K]) hw->iflags|=(1<<4);
	if (clock[CLKD8K]) hw->iflags|=(1<<3);

	//Fire interrupts if enabled
	ien=REG(R_INTCTLLO)+(REG(R_INTCTLMI)<<8);
//	if (ien&hw->iflags) printf("Firing int because of iflags 0x%X\n", (ien&hw->iflags));
	if ((ien&hw->iflags)&(1<<0)) Int6502(R, INT_IRQ, IRQVECT_FP);
	if ((ien&hw->iflags)&(1<<1)) Int6502(R, INT_IRQ, IRQVECT_SPI);
	if ((ien&hw->iflags)&(1<<2)) Int6502(R, INT_IRQ, IRQVECT_SPU);
	if ((ien&hw->iflags)&(1<<3)) Int6502(R, INT_IRQ, IRQVECT_FROSCD8K);
	if ((ien&hw->iflags)&(1<<4)) Int6502(R, INT_IRQ, IRQVECT_FROSCD2K);
	if ((ien&hw->iflags)&(1<<7)) Int6502(R, INT_IRQ, IRQVECT_T0);
	if ((ien&hw->iflags)&(1<<10)) Int6502(R, INT_IRQ, IRQVECT_TBL);
	if ((ien&hw->iflags)&(1<<11)) Int6502(R, INT_IRQ, IRQVECT_TBH);
	if ((ien&hw->iflags)&(1<<13)) Int6502(R, INT_IRQ, IRQVECT_T1);

	//Fire NMI
	if (REG(R_NMICTL)&0x80) {
//		if (REG(R_NMICTL)&nmiTrigger)printf("Firing NMI; nmitrigger=0x%X\n", nmiTrigger);
		//Should be edge triggred. The timer is. An implementation of lv may not be.
		if (REG(R_NMICTL)&nmiTrigger) Int6502(R, INT_NMI, 0);
		hw->nmiflags=nmiTrigger;
	}

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



