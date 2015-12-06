/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"


#define I2C_IDLE 0
#define I2C_B0 1
#define I2C_B7 8
#define I2C_ACK 9


I2cBus *i2cInit() {
	I2cBus *b=malloc(sizeof(I2cBus));
	memset(b, 0, sizeof(I2cBus));
	return b;
}

void i2cFree(I2cBus *b) {
	free(b);
}

int i2cHandle(I2cBus *b, int scl, int sda) {
	int ret=b->oldOut;
//	printf("I2c state: %d\n", b->state);
	if (b->oldScl && scl && !b->oldSda && sda) {
		//Stop condition.
		b->state=I2C_IDLE;
		ret=1;
//		printf("I2C: stop\n");
	} else if (b->oldScl && scl && b->oldSda && !sda) {
		//Start condition
		b->state=I2C_B0;
		b->byteCnt=0;
		b->dirOut=0;
//		printf("I2C: start\n");
		ret=1;
	} else if (!b->oldScl && scl && b->oldSda == sda && b->state!=I2C_IDLE) {
		ret=1;
		//Clock up: bit is clocked in or out.
		if (b->dirOut) {
			if (b->state!=I2C_ACK) {
				if (b->state==I2C_B0) {
					//Fetch new byte from dev
					if (b->dev[b->adr/2]) {
						b->byte=b->dev[b->adr/2]->readCb(b->dev[b->adr/2]->dev,b->byteCnt);
					} else {
						//No such dev
						b->byte=0xff;
					}
				}
				if (!(b->byte&0x80)) ret=0;
				b->byte<<=1;
				b->state++;
			} else {
				//ToDo: read ack, send to dev
				b->byteCnt++;
				b->state=I2C_B0;
			}
		} else {
			if (b->state!=I2C_ACK) {
				//Receiving bit of byte
				b->byte<<=1; if (sda) b->byte|=0x1;
				b->state++;
			} else {
				//Byte is in.
//				printf("I2C: got byte %d val 0x%02X\n", b->byteCnt, b->byte);
				if (b->byteCnt==0) {
					//Address byte
					b->adr=b->byte;
					if ((b->adr&1)) b->dirOut=1;
					if (b->dev[b->adr/2]) ret=0; else ret=1; //Ack if dev is available.
				} else {
					//Send byte to dev, grab ack and send to host
					if (b->dev[b->adr/2]) {
						ret=(b->dev[b->adr/2]->writeCb(b->dev[b->adr/2]->dev, b->byteCnt, b->byte))?0:1;
					} else {
						ret=1;
					}
				}
				b->byteCnt++;
				b->state=I2C_B0;
			}
		}
	}
	b->oldScl=scl;
	b->oldSda=sda;
	b->oldOut=ret;
	return ret;
}

void i2cAddDev(I2cBus *b, I2cDev *dev, int addr) {
	b->dev[addr/2]=dev;
}
