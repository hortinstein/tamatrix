#ifndef I2C_H
#define I2C_H
#include <stdint.h>

typedef int (*I2cDevWriteCb)(void *dev, int byteNo, uint8_t byte);
typedef int (*I2cDevReadCb)(void *dev, int byteNo);

typedef struct {
	I2cDevWriteCb writeCb;
	I2cDevReadCb readCb;
	void *dev;
} I2cDev;


typedef struct {
	int state;
	uint8_t byte;
	int dirOut;
	int oldScl;
	int oldSda;
	int adr;
	int byteCnt;
	int oldOut;
	I2cDev *dev[128];
} I2cBus;


I2cBus *i2cInit();
void i2cFree(I2cBus *b);
int i2cHandle(I2cBus *b, int scl, int sda);
void i2cAddDev(I2cBus *b, I2cDev *dev, int addr);

#endif