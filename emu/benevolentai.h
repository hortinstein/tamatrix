#ifndef BENEVOLENTAI_H
#define BENEVOLENTAI_H

#include "lcd.h"

void benevolentAiInit();
int benevolentAiRun(Display *lcd, int mspassed);
int benevolentAiMacroRun(char *name);
void benevolentAiReqIrComm(int type);
void benevolentAiAckIrComm(int type);

#endif