#include "lcdmatch.h"

int lcdmatch(Display *lcd, const unsigned char *screen) {
	int p=0;
	int x=0;
	int y=0;
	while (y<32) {
		if (screen[p]&0x80) {
			x+=(screen[p]&0x7f);
		} else {
//			lcd->p[y][x]=3;
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
