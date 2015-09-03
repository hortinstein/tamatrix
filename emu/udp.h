#ifndef UDP_H
#define UDP_H

#include "lcd.h"

void udpInit(char *hostname);
void udpSendDisplay(Display *d);
void udpSendIr(char *data, int len);
int udpTick();

#endif
