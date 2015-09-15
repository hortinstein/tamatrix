#include <stdio.h>
#include <string.h>
#include "udp.h"

static int oldLight=0;
static int seenLight=0;
static int ticks=0;
static int hiTime=0;
static int bit=0;
static int val=0;
static int currClkTick=0;

static int recvActive=0;
static char sendData[32];
//SendPos: -2: inactive, -1 - sending sync pulse, 0-n: sending bit
static int sendPos=-2, sendLen, sendTick;
static int sendStartPulse=0;

static char recvData[32];
static int recvPos=-1;
static int totalTicks;
static int recvStartPulse;

#define IRTICK_MAX ((16000000/38000))

void irRecv(char *data, int len, int startPulseLen) {
	if (len>32) return;
	fprintf(stderr, "UDP->Tama, len=%d. Curr: sendPos=%d. Ticks since send: %d\n", len, sendPos, ticks);
	memcpy(sendData, data, len);
	sendStartPulse=startPulseLen;
	sendLen=len;
	sendPos=-1; 
	sendTick=0;
}


//Called when the IR led is active.
void irActive(int isOn) {
	if (isOn) seenLight=1;
}

//IR timing
#define TICKS_HI_IDLE_TH	160 //threshold for start
#define TICKS_LO_ZERO_TH	17 //threshold between 0 and 1
#define TICKS_SENDHI		10
#define TICKS_SENDLOZERO	12
#define TICKS_SENDLOONE		24
#define TICKS_START_LO		42
#define TICKS_END_HI		24
#define TICKS_END_HI_TH		21

//This is called every [gran] clock cycles and returns the output of the IR receiver
int irTick(int noticks, int *irNX) {
	int b;
	currClkTick+=noticks;
	if (currClkTick<IRTICK_MAX) return recvActive;
	currClkTick-=IRTICK_MAX;

	if (sendPos!=-2) {
		sendTick++;
		if (sendPos==-1) {
			if (sendTick<sendStartPulse) {
				recvActive=1;
			} else if (sendTick==sendStartPulse) {
				recvActive=0;
			} else if (sendTick==sendStartPulse+TICKS_START_LO) {
				recvActive=1;
				sendPos=0;
				sendTick=0;
			}
		} else {
			if (sendPos!=(sendLen*8)) {
				if (sendTick==TICKS_SENDHI) recvActive=0;
			} else {
				if (sendTick==TICKS_END_HI) recvActive=0;
			}
			b=sendData[sendPos/8]&(1<<(sendPos&7));
			if ((b && (sendTick==(TICKS_SENDHI+TICKS_SENDLOONE))) || ((!b) && (sendTick==(TICKS_SENDHI+TICKS_SENDLOZERO)))) {
				sendTick=0;
				sendPos++;
//				printf("Irsend: pos %d\n", sendPos);
				if (sendPos<(sendLen*8)+1) {
					recvActive=1;
				} else {
					//We're done sending.
					recvActive=0;
					sendPos=-2;
				}
			}
		}
		fprintf(stderr, "Sendpos %d (B%d b%d) tick %d recv %d\n", sendPos, sendPos>>3, sendPos&7, sendTick, recvActive);
	}

//	seenLight=recvActive; //HACK
	if (oldLight!=seenLight) {
		if (oldLight==1) {
			// IR goes high->low
			hiTime=ticks;
			if (ticks>TICKS_END_HI_TH && recvPos!=-1) {
				totalTicks+=ticks;
				fprintf(stderr, "Transmit (Tama->udp) ended: %d bytes. Took %d ticks.\n", recvPos, totalTicks);
				udpSendIr(recvData, recvPos, recvStartPulse);
				recvPos=-1;
				*irNX+=(totalTicks*IRTICK_MAX);
				ticks=0;
			}
		} else {
			// IR goes low->hi
			fprintf(stderr, "Hi %02d lo %02d (%d) bit %d byte %d\n", hiTime, ticks, (ticks>TICKS_LO_ZERO_TH)?1:0,
					bit, recvPos);
			if (hiTime>TICKS_HI_IDLE_TH) {
				bit=0; val=0;
				recvPos=0;
				totalTicks=hiTime;
				recvStartPulse=hiTime;
//				fprintf(stderr, "IR: start\n");
			} else if (recvPos!=-1) {
				val>>=1;
				if (ticks>TICKS_LO_ZERO_TH) {
					val|=0x80;
					totalTicks+=TICKS_SENDHI+TICKS_SENDLOONE;
				} else {
					totalTicks+=TICKS_SENDHI+TICKS_SENDLOZERO;
				}
				bit++;
				if (bit==8) {
					recvData[recvPos++]=val;
					if (recvPos==32) recvPos=31; //don't overflow
//					fprintf(stderr, "IR: %02X\n", val);
					val=0; bit=0;
				}
			}
		}
		ticks=0;
		oldLight=seenLight;
	}
	ticks++;

	seenLight=0;
	return recvActive;
}
