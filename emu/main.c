#include <sys/time.h>
#include <time.h>
#include "stdio.h"
#include "tamaemu.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>



//tama lcd is 48x32
void displayDram(uint8_t *ram, int sx, int sy) {
	char *icons[]={"INFO", "FOOD", "TOILET", "DOORS", "FIGURE",
				"TRAINING", "MEDICAL", "IR", "ALBUM", "ATTENTION"};
	int x, y;
	int b, p;
	char grays[]=" '*M";
	printf("\033[45;1H\033[1J\033[1;1H");
	for (y=0; y<sy+1; y++) {
		for (x=0; x<sx; x++) {
			if (y>=16) {
				p=x+(sy-y-1)*sx;
			} else {
				p=x+(sy-(15-y)-1)*sx;
			}
			b=ram[p/4];
			b=(b>>((3-(p&3))*2))&3;
			putchar(grays[b]);
			putchar(grays[b]);
		}
		putchar('\n');
	}
	printf(">>> ");
	for (x=19; x<29; x++) {
		b=ram[x/4];
		b=(b>>((3-(x&3))*2))&3;
		if (b!=0) printf("%s ", icons[x-19]);
	}
	printf("<<<\n");
}

Tamagotchi *tama;


void sigintHdlr(int sig)  {
	if (tama->cpu->Trace) exit(1);
	tama->cpu->Trace=1;
}


int getKey() {
	char buff[100];
	fd_set fds;
	struct timeval tv;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	select(1, &fds, NULL, NULL, &tv);
	if (FD_ISSET(0, &fds)) {
		fgets(buff, 99, stdin);
		return atoi(buff);
	} else {
		return 0;
	}
}


#define FPS 10

int main(int argc, char **argv) {
	unsigned char **rom;
	long us;
	int k;
	struct timespec tstart, tend;
	signal(SIGINT, sigintHdlr);
	rom=loadRoms();
	tama=tamaInit(rom);
	while(1) {
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		tamaRun(tama, FCPU/FPS);
		displayDram(tama->dram, tama->lcd.sizex, tama->lcd.sizey);
		tamaDumpHw(tama->cpu);
		clock_gettime(CLOCK_MONOTONIC, &tend);
		us=(tend.tv_nsec-tstart.tv_nsec)/1000;
		us+=(tend.tv_sec-tstart.tv_sec)*1000000L;
		us=(1000000L/FPS)-us;
		printf("Time left in frame: %d us\n", us);
		if (us>0) usleep(us);
		k=getKey();
		if (k>0) {
			tamaPressBtn(tama, k-1);
		}

	}
}
