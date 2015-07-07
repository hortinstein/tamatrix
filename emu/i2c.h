#ifndef I2C_H
#define I2C_H
#include <stdint.h>


typedef struct {
	int state;
	uint8_t byte;
	int dirOut;
	int oldScl;
	int oldSda;
	int adr;
	int byteCnt;
} I2cBus;


I2cBus *i2cInit();
void i2cFree(I2cBus *b);
int i2cHandle(I2cBus *b, int scl, int sda);

#endif