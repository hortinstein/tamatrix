#include "i2ceeprom.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int i2ceepromWrite(void *dev, int byteNo, uint8_t byte) {
	I2cEeprom *e=(I2cEeprom *)dev;
	if (byteNo==1) {
		e->adr=byte;
	} else {
		e->mem[e->adr++]=byte;
		e->adr&=0xff;
	}
	return 1;
}

int i2ceepromRead(void *dev, int byteNo) {
	I2cEeprom *e=(I2cEeprom *)dev;
	int r;
	r=e->mem[e->adr++];
	e->adr&=0xff;
	return r;
}

I2cEeprom *i2ceepromInit(char *filename) {
	int x;
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
		write(e->fd, emem, 256);
	}
	e->mem=mmap(NULL, 256, PROT_READ|PROT_WRITE, MAP_SHARED, e->fd, 0);
	if (e->mem==NULL) {
		perror("mmap");
		exit(1);
	}
	return e;
}

