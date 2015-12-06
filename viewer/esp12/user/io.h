#ifndef IO_H
#define IO_H

void ICACHE_FLASH_ATTR ioFade(int from, int to, int time);
void ICACHE_FLASH_ATTR ioPwm(int ena);
void ioInit(void);

#endif