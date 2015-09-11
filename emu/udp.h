#ifndef UDP_H
#define UDP_H

#include "lcd.h"

void udpInit(char *hostname);
void udpSendDisplay(Display *d);
void udpSendIr(char *data, int len);
void udpTick();
void udpSendIrstartReq(int type);
void udpSendIrstartAck(int type);
void udpExit();


#endif
