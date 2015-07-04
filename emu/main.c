#include "stdio.h"
#include "tamaemu.h"
#include <signal.h>

void displayDram(uint8_t *ram) {
	int x, y;
	int b;
	char grays[]=" .*M";
	printf("\033[1;1H");
	for (y=0; y<32; x++) {
		for (x=0; x<64; y++) {
			b=ram[x+(y/4)*32];
			b=(b>>((y&3)*2))&3;
			putchar(grays[b]);
			putchar(grays[b]);
		}
		putchar('\n');
	}
}

Tamagotchi *tama;


void sigintHdlr(int sig)  {
	tama->cpu->Trace=1;
}

int main(int argc, char **argv) {
	unsigned char **rom;
	signal(SIGINT, sigintHdlr);
	rom=loadRoms();
	tama=tamaInit(rom);
	while(1) {
		tamaRun(tama, 8000000L/50);
		displayDram(tama->dram);
	}
}
