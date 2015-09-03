#include <sys/time.h>
#include <time.h>
#include "stdio.h"
#include "tamaemu.h"
#include "udp.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

#include "lcd.h"
#include "benevolentai.h"
#include "udp.h"

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
		return buff[0];
	} else {
		return 0;
	}
}


#define FPS 10

int main(int argc, char **argv) {
	unsigned char **rom;
	long us;
	int k, i;
	int speedup=0;
	int stopDisplay=0;
	int t=0;
	char *eeprom="tama.eep";
	char *host="127.0.0.1";
	struct timespec tstart, tend;
	Display display;
	int err=0;

	for (i=1; i<argc; i++) {
		if (strcmp(argv[i],"-h")==0 && argc>i+1) {
			i++;
			host=argv[i];
		} else if (strcmp(argv[i],"-e")==0 && argc>i+1) {
			i++;
			eeprom=argv[i];
		} else {
			printf("Unrecognized option - %s\n", argv[i]);
			err=1;
			break;
		}
	}

	if (err) {
		printf("Usage: %s [options]\n", argv[0]);
		printf("-h host - change tamaserver host address (def 127.0.0.1)\n");
		printf("-e eeprom.eep - change eeprom file (def tama.eep)\n");
		exit(0);
	}

	signal(SIGINT, sigintHdlr);
	rom=loadRoms();
	tama=tamaInit(rom, eeprom);
	benevolentAiInit();
	udpInit(host);
	while(1) {
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		tamaRun(tama, FCPU/FPS-1);
		lcdRender(tama->dram, tama->lcd.sizex, tama->lcd.sizey, &display);
		i=udpTick();
		if (i) benevolentAiReqIrComm();
		k=benevolentAiRun(&display, 1000/FPS);
		if (!speedup || (t&15)==0) {
			lcdShow(&display);
			udpSendDisplay(&display);
			tamaDumpHw(tama->cpu);
			benevolentAiDump();
		}
		if ((k&8)) {
			//If anything interesting happens, make a LCD dump.
			lcdDump(tama->dram, tama->lcd.sizex, tama->lcd.sizey, "lcddump.lcd");
			if (stopDisplay) {
				tama->cpu->Trace=1;
				speedup=0;
			}
		}
		if (k&1) tamaPressBtn(tama, 0);
		if (k&2) tamaPressBtn(tama, 1);
		if (k&4) tamaPressBtn(tama, 2);
		clock_gettime(CLOCK_MONOTONIC, &tend);
		us=(tend.tv_nsec-tstart.tv_nsec)/1000;
		us+=(tend.tv_sec-tstart.tv_sec)*1000000L;
		us=(1000000L/FPS)-us;
//		printf("Time left in frame: %d us\n", us);
		if (!speedup && us>0) usleep(us);
		k=getKey();
		if (k=='1') tamaPressBtn(tama, 0);
		if (k=='2') tamaPressBtn(tama, 1);
		if (k=='3') tamaPressBtn(tama, 2);
		if (k=='s') speedup=!speedup;
		if (k=='d') stopDisplay=!stopDisplay;
		t++;
	}
}
