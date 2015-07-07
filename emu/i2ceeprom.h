#ifndef I2CEEPROM_H
#define I2CEEPROM_H

#include "i2c.h"


typedef struct {
	I2cDev i2cdev;
	int adr;
	int fd;
	char *mem;
} I2cEeprom;

I2cEeprom *i2ceepromInit(char *filename);

#endif
