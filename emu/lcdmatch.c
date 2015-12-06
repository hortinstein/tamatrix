/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include "lcdmatch.h"


int lcdmatchOffs(Display *lcd, const unsigned char *screen, int yoff) {
	int p=0;
	int x=0;
	int y=yoff;
	while (y<32) {
		if (screen[p]&0x80) {
			x+=(screen[p]&0x7f);
		} else {
//			lcd->p[y][x]=3;
			if (y<0 && y>32) return 0;
			if (screen[p]=='.' && lcd->p[y][x]==3) return 0;
			if (screen[p]=='X' && lcd->p[y][x]!=3) return 0;
			x++;
		}
		while(x>=48) {
			x-=48;
			y+=1;
		}
		p++;
	}
	return 1;
}

int lcdmatch(Display *lcd, const unsigned char *screen) {
	return lcdmatchOffs(lcd, screen, 0);
}

int lcdmatchMovable(Display *lcd, const unsigned char *screen, int bot, int top) {
	int i;
	for (i=bot; i<top; i++) {
		if (lcdmatchOffs(lcd, screen, i)) return 1;
	}
	return 0;
}
