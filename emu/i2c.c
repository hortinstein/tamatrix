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
	int ret=sda;
	if (b->oldScl && scl && !b->oldSda && sda) {
		//Stop condition.
		b->state=I2C_IDLE;
	} else if (b->oldScl && scl && b->oldSda && !sda) {
		//Start condition
		b->state=I2C_B0;
		b->byteCnt=0;
		b->dirOut=0;
		printf("I2C: start\n");
	} else if (!b->oldScl && scl && b->oldSda == sda && b->state!=I2C_IDLE) {
		//Clock bit
		if (b->dirOut) {
			if (b->state!=I2C_ACK) {
				if (b->state==I2C_B0) {
					//ToDo: grab byte from dev
					b->byte=0xff;
				}
				if (!(b->byte&1)) ret=0;
				b->byte>>=1;
				b->state++;
			} else {
				//ToDo: read ack, send to dev
				b->byteCnt++;
				b->state=I2C_B0;
			}
		} else {
			if (b->state!=I2C_ACK) {
				b->byte>>=1; if (sda) b->byte|=0x80;
				b->state++;
			} else {
				//Byte is in.
				if (b->byteCnt==0) {
					b->adr=b->byte;
					if (!(b->adr&0x80)) b->dirOut=1;
				}
				printf("I2C: got byte %d val 0x%02X\n", b->byteCnt, b->byte);
				//Send byte to dev, grab ack and send to host
				ret=0; //HACK: always ack.
				b->byteCnt++;
				b->state=I2C_B0;
			}
		}
		b->state++;
	}
	b->oldScl=scl;
	b->oldSda=sda;
	return ret;
}
