/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <string.h>
#include "udp.h"


/*
Yes, this code is very sucky, but it cost me half an eternity to get working. Don't try to 'fix' it
unless you know what you are doing!
*/

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

static char recvData[32];
static int recvPos=-1;
static int totalTicks;

#define IRTICK_MAX ((16000000/38000))

void irRecv(char *data, int len) {
	if (len>32) return;
	fprintf(stderr, "Got IR data from UDP, len=%d. Curr: sendPos=%d\n", len, sendPos);
	memcpy(sendData, data, len);
	sendLen=len;
	sendPos=-1; 
	sendTick=0;
}


//Called when the IR led is active.
void irActive(int isOn) {
	if (isOn) seenLight=1;
}

//IR timing
#define TICKS_HI_IDLE_TH	150
#define TICKS_LO_ZERO_TH	17 //threshold between 0 and 1
#define TICKS_SENDHI		10
#define TICKS_SENDLOZERO	12
#define TICKS_SENDLOONE		24
#define TICKS_START_HI		180
#define TICKS_START_LO		42
#define TICKS_END_HI		24
#define TICKS_END_HI_TH		21

//This is called every 2048 clock cycles and returns the output of the IR receiver
int irTick(int noticks, int *irNX) {
	int b;
	currClkTick+=noticks;
	if (currClkTick<IRTICK_MAX) return recvActive;
	currClkTick-=IRTICK_MAX;

	if (sendPos!=-2) {
		sendTick++;
		if (sendPos==-1) {
			if (sendTick<TICKS_START_HI) {
				recvActive=1;
			} else if (sendTick==TICKS_START_HI) {
				recvActive=0;
			} else if (sendTick==TICKS_START_HI+TICKS_START_LO) {
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
	}

//	seenLight=recvActive; //HACK
	if (oldLight!=seenLight) {
		if (oldLight==1) {
			// IR goes high->low
			hiTime=ticks;
			if (ticks>TICKS_END_HI_TH && recvPos>0) {
				//Okay, we just collected the IR data sent by this tama and sent it to the server. That
				//will send it to another tama, which will then replay it to the software. The problem is
				//that that will take some time, while in reality, the other tama would've been done receiving
				//when this one is done sending. To compensate for that, after receiving IR data, we will stop
				//the execution of this tama for as long as it took to send the IR stream. That way, the
				//situation is back to what it would have been in real life as soon as execution resumes:
				//this tama just finished sending the data and the other tama just finished receiving it.
				//
				//Actually, we need to wait twice as long: we just did tama1->AI, but we need to compensate
				//for AI->tama2 and tama2->AI before we start receiving. Somehow, entering a factor 2 doesn't quite
				//work though... we'll stick with 1.1 for now.
				udpSendIr(recvData, recvPos, 0);
				recvPos=-1;
				totalTicks+=ticks;
				*irNX+=(totalTicks*IRTICK_MAX)*1.1;
			}
		} else {
			// IR goes low->hi
			fprintf(stderr, "Hi %02d lo %02d (%d) bit %d byte %d\n", hiTime, ticks, (ticks>TICKS_LO_ZERO_TH)?1:0,
					bit, recvPos);
			if (hiTime>TICKS_HI_IDLE_TH) {
				bit=0; val=0;
				recvPos=0;
				totalTicks=0;
//				fprintf(stderr, "IR: start\n");
			} else if (recvPos!=-1) {
				val>>=1;
				if (ticks>TICKS_LO_ZERO_TH) val|=0x80;
				bit++;
				if (bit==8) {
					recvData[recvPos++]=val;
					if (recvPos==32) recvPos=31; //don't overflow
//					fprintf(stderr, "IR: %02X\n", val);
					val=0; bit=0;
				}
			}
		}
		totalTicks+=ticks;
		ticks=0;
		oldLight=seenLight;
	}
	ticks++;
	seenLight=0;
	return recvActive;
}
