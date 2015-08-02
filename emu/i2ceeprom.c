#include "i2ceeprom.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//Q&D code to emulate a 64K (or less) EEPROM like the 24c16


int i2ceepromWrite(void *dev, int byteNo, uint8_t byte) {
	I2cEeprom *e=(I2cEeprom *)dev;
	if (byteNo==1) {
		e->adr=byte<<8;
	} else if (byteNo==2) {
		e->adr|=byte;
	} else {
		int page=e->adr&0xFFE0;
//		printf("I2CEEprom write: (%d) addr %02X val %02X\n", byteNo-2, e->adr, byte);
		e->mem[e->adr]=byte;
		e->adr++;
		//Simulate in-page rollover
		e->adr=page|(e->adr&0x1F);
	}
	return 1;
}

int i2ceepromRead(void *dev, int byteNo) {
	int r;
	I2cEeprom *e=(I2cEeprom *)dev;
	r=e->mem[e->adr];
//	printf("I2cEEprom read: addr %02x val %02x\n", e->adr, r);
	e->adr++;
	e->adr&=0xffff;
	return r;
}

I2cEeprom *i2ceepromInit(char *filename) {
	int x, i;
	I2cEeprom *e=malloc(sizeof(I2cEeprom));
	e->i2cdev.writeCb=i2ceepromWrite;
	e->i2cdev.readCb=i2ceepromRead;
	e->i2cdev.dev=e;
	e->fd=open(filename, O_RDWR);
	if (e->fd<=0) {
		char emem[256];
		for (x=0; x<256; x++) emem[x]=0xff;
		e->fd=open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
		if (e->fd<=0) {
			perror(filename);
			exit(1);
		}
		for (i=0; i<256; i++) write(e->fd, emem, 256);
	}
	e->mem=mmap(NULL, 65536, PROT_READ|PROT_WRITE, MAP_SHARED, e->fd, 0);
	if (e->mem==NULL) {
		perror("mmap");
		exit(1);
	}
	return e;
}

