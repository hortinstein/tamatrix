#include "lcd.h"
#include <stdio.h>


void lcdShow(Display *lcd) {
	int i;
	int x,y;
	char *icons[]={"INFO", "FOOD", "TOILET", "DOORS", "FIGURE",
				"TRAINING", "MEDICAL", "IR", "ALBUM", "ATTENTION"};
	char grays[]=" '*M";
	printf("\033[45;1H\033[1J\033[1;1H");
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			putchar(grays[lcd->p[y][x]&3]);
			putchar(grays[lcd->p[y][x]&3]);
		}
		putchar('\n');
	}
	printf(">>> ");
	i=lcd->icons;
	for (x=0; x<10; x++) {
		if (i&1) printf("%s ", icons[x]);
		i>>=1;
	}
	printf("<<<\n");
}

//tama lcd is 48x32
void lcdRender(uint8_t *ram, int sx, int sy, Display *lcd) {
	int x, y;
	int b, p;
	for (y=0; y<sy+1; y++) {
		for (x=0; x<sx; x++) {
			if (y>=16) {
				p=x+(sy-y-1)*sx;
			} else {
				p=x+(sy-(15-y)-1)*sx;
			}
			b=ram[p/4];
			b=(b>>((3-(p&3))*2))&3;
			if (y<32 && x<48) lcd->p[y][x]=b;
		}
	}
	
	y=1;
	lcd->icons=0;
	for (x=19; x<29; x++) {
		b=ram[x/4];
		b=(b>>((3-(x&3))*2))&3;
		if (b!=0) lcd->icons|=y;
		y<<=1;
	}
}
