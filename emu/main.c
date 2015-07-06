#include "stdio.h"
#include "tamaemu.h"
#include <signal.h>
#include <stdlib.h>

//tama lcd is 48x32
void displayDram(uint8_t *ram, int sx, int sy) {
	int x, y;
	int b, p;
	char grays[]=" '*M";
	printf("\033[1;1H");
	for (y=0; y<sy; y++) {
		for (x=0; x<sx; x++) {
			p=x+(sy-y-1)*sx;
			b=ram[p/4];
			b=(b>>((3-(p&3))*2))&3;
			putchar(grays[b]);
			putchar(grays[b]);
		}
		putchar('\n');
	}
}

Tamagotchi *tama;


void sigintHdlr(int sig)  {
	if (tama->cpu->Trace) exit(1);
	tama->cpu->Trace=1;
}

int main(int argc, char **argv) {
	unsigned char **rom;
	signal(SIGINT, sigintHdlr);
	rom=loadRoms();
	tama=tamaInit(rom);
	while(1) {
		tamaRun(tama, 8000000L/10);
		displayDram(tama->dram, tama->lcd.sizex, tama->lcd.sizey);
	}
}
