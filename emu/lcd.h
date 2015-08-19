#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define ICON_INFO (1<<0)
#define ICON_FOOD (1<<1)
#define ICON_TOILET (1<<2)
#define ICON_DOORS (1<<3)
#define ICON_FIGURE (1<<4)
#define ICON_TRAINING (1<<5)
#define ICON_MEDICAL (1<<6)
#define ICON_IR (1<<7)
#define ICON_ALBUM (1<<8)
#define ICON_ATTENTION (1<<9)


typedef struct {
	char p[32][48];
	int icons;
} Display;

void lcdShow(Display *lcd);
void lcdRender(uint8_t *ram, int sx, int sy, Display *lcd);
void lcdDump(uint8_t *ram, int sx, int sy, char *fn);


#endif