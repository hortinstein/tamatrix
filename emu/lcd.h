#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define ICON_INFO (1<<0)
#define ICON_FOOD (1<<0)
#define ICON_TOILET (1<<0)
#define ICON_DOORS (1<<0)
#define ICON_FIGURE (1<<0)
#define ICON_TRAINING (1<<0)
#define ICON_MEDICAL (1<<0)
#define ICON_IR (1<<0)
#define ICON_ALBUM (1<<0)
#define ICON_ATTENTION (1<<0)


typedef struct {
	char p[32][48];
	int icons;
} Display;

void lcdShow(Display *lcd);
void lcdRender(uint8_t *ram, int sx, int sy, Display *lcd);

#endif