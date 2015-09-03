#include "benevolentai.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcdmatch.h"
#include "screens.h"


typedef struct {
	char *name;
	char *code;
} Macro;


int curMacro=-1;
int macroPos=0;
int waitTimeMs=0;
int cmd, arg;
int state=0;

int hunger=-1;
int happy=-1;
int oldIcon=-1;
int iconAttempts=0;

#define ST_IDLE 0
#define ST_NEXT 1
#define ST_ICONSEL 2


static Macro macros[]={
	{"feedmeal", "s2,p2,p2,p2,w90,p3,p3"},
	{"feedsnack", "s2,p2,p1,p2,p2,w90,p3,p3"},
	{"train", "s6,p2,p2,w40"},
	{"medicine", "s7,p2,w40"},
	{"loadeep", "w10,p2,p2,w20"},
	{"updvars", "s1,p2,w10,p1,w10,m,p3"},
	{"toilet", "w10,s3,p2,p2,w50"},
	{"toiletpraise", "w10,s3,p2,p2,w50,s6,p2,p1,p2,w50"},
	{"lightoff", "p3,w20"},
	{"playstb", "s4,p2,p2,w90,p2"},
	{"playjump", "s4,p2,p2,w90,p1,p2"},
	{"exitgame", "p3,w80"},
	{"stbshoot", "p2"},
	{"dojump", "p2"},
	{"tst", "s8"},
	{"", ""}
};


void benevolentAiReqIrComm() {

}

int benevolentAiMacroRun(char *name) {
	int i=0;
	if (state!=ST_IDLE) return 0;
	while (strcasecmp(macros[i].name, name)!=0 && macros[i].name[0]!=0) i++;
	if (macros[i].name[0]==0) {
		printf("Macro %s not found. Available macros:\n", name);
		i=0;
		while(macros[i].name[0]!=0) printf(" - %s\n", macros[i++].name);
		return 0;
	}
	macroPos=0;
	curMacro=i;
	state=ST_NEXT;
	return 1;
}

void benevolentAiInit() {
	state=ST_IDLE;
	benevolentAiMacroRun("loadeep");
}


//Returns a bitmap of buttons if the macro requires pressing one, or -1 if no macro is running.
int macroRun(Display *lcd, int mspassed) {
	int i;
	if (state==ST_IDLE) return -1;
	waitTimeMs-=mspassed;
	if (waitTimeMs>0) return 0;
	waitTimeMs=0;

	if (state==ST_NEXT) {
		if (macros[curMacro].code[macroPos]==0) {
			//End of macro.
			state=ST_IDLE;
			return -1;
		} else {
			cmd=macros[curMacro].code[macroPos++];
			arg=atoi(&macros[curMacro].code[macroPos]);
			//Find ',' or end of string
			while (macros[curMacro].code[macroPos]!=',' && macros[curMacro].code[macroPos]!=0) macroPos++;
			//Skip past ','
			if (macros[curMacro].code[macroPos]==',') macroPos++;
			if (cmd=='p') {
				//Press a button
				waitTimeMs=300;
				return (1<<(arg-1));
			} else if (cmd=='w') {
				//Wait x deciseconds
				waitTimeMs=arg*100;
				return 0;
			} else if (cmd=='s') {
				//Select icon...
				state=ST_ICONSEL;
				waitTimeMs=0;
				oldIcon=-1;
				iconAttempts=0;
				return 0;
			} else if (cmd=='m') {
				//Assume we're on the hunger/happy screen; we can now measure the amount of heart filled.
				hunger=0; happy=0;
				for (i=0; i<5; i++) {
					if (lcd->p[10][i*10+6]==3) hunger++;
					if (lcd->p[26][i*10+6]==3) happy++;
				}
			} else {
				printf("Huh? Unknown macro cmd %c (macro %d pos %d)\n", cmd, curMacro, macroPos);
				exit(0);
			}
		}
	} else if (state==ST_ICONSEL) {
		iconAttempts++;
		if (iconAttempts==15) state=ST_NEXT; //Bail out.
		if (lcd->icons&(1<<(arg-1))) {
			state=ST_NEXT;
		} else {
			waitTimeMs=300;
			if (oldIcon==lcd->icons) {
				//Icon didn't change. Maybe press 'back'?
				oldIcon=-1;
				return (1<<2);
			} else {
				//Not there yet. Select next icon.
				return (1<<0);
			}
		}
	}
	return 0;
}


static int getDarkPixelCnt(Display *lcd) {
	int c=0;
	int x, y;
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			if (lcd->p[y][x]==3) c++;
		}
	}
	return c;
}

#define BA_IDLE 0
#define BA_CHECKFOOD 1
#define BA_FEED 2
#define BA_RECHECKFOOD 3
#define BA_RECHECKLESSHUNGRY 4
#define BA_STB 5
#define BA_JUMP 6

int baTimeMs;
int baState=BA_IDLE;
int oldPxCnt;
int timeout=0;

int oldHunger;
int oldHappy;

void benevolentAiDump() {
	char *bastates[]={"idle", "checkfood", "feed", "recheckfood", "rechecklesshungry", "shootthebug", "jump"};
	printf("Current macro: ");
	if (state==ST_IDLE) {
		printf("None.\n");
	} else {
		printf("%s, at cmd %c arg %d\n", macros[curMacro].name, cmd, arg);
	}
	printf("Benevolent AI state: %s\n", bastates[baState]);
}


int benevolentAiRun(Display *lcd, int mspassed) {
	int i;
	int r=macroRun(lcd, mspassed);
	if (r!=-1) return r;
	baTimeMs+=mspassed;

	if (timeout!=0) {
		timeout-=mspassed;
		if (timeout<=0) {
			timeout=0;
			baState=BA_IDLE;
		}
	}

	if (baState==BA_IDLE) {
		if (lcdmatch(lcd, screen_poopie1) || lcdmatch(lcd, screen_poopie2)|| lcdmatch(lcd, screen_poopie3)) {
			benevolentAiMacroRun("toilet");
		} else if (lcdmatchMovable(lcd, screen_pooping1, -16, 2) || lcdmatchMovable(lcd, screen_pooping2, -16, 2)) {
			benevolentAiMacroRun("toiletpraise");
		} else if (lcdmatch(lcd, screen_sick1) || lcdmatch(lcd, screen_sick2)){
			benevolentAiMacroRun("medicine");
		} else if (lcdmatchMovable(lcd, screen_sleep1, -16, 2) || lcdmatchMovable(lcd, screen_sleep2, -2, 2)) {
			benevolentAiMacroRun("lightoff");
			baTimeMs=0;
		} else if (getDarkPixelCnt(lcd)>1000) {
			//We turned off the light.
			baTimeMs=0; //Don't wake up to check info
		} else if (lcdmatch(lcd, screen_alert)){
			benevolentAiMacroRun("train");
		} else if (baTimeMs>300*1000) {
			//We need to check for health etc
			baTimeMs=0;
			baState=BA_CHECKFOOD;
		} else {
			i=getDarkPixelCnt(lcd);
			if (i<(oldPxCnt-10) || i>(oldPxCnt+10)) {
				printf("Pix cnt %d, was %d\n", i, oldPxCnt);
				oldPxCnt=(i+oldPxCnt)/2;
				return 8;
			}
		}
	} else if (baState==BA_CHECKFOOD) {
		benevolentAiMacroRun("updvars");
		baState=BA_FEED;
	} else if (baState==BA_FEED) {
		oldHunger=hunger;
		oldHappy=happy;
		if (hunger<4) {
			benevolentAiMacroRun("feedmeal");
			baState=BA_RECHECKFOOD;
		} else if (happy<4) {
			//Bug: If Tama is congested, it doesn't want to go anywhere. Putting it into the feedsnack/playstb/...
			//functions make it do nothing. Timeout will catch this tho'.
			i=rand()%3;
			if (i==0) {
				benevolentAiMacroRun("feedsnack");
				baState=BA_RECHECKFOOD;
			} else if (i==1) {
				baState=BA_STB;
				benevolentAiMacroRun("playstb");
				timeout=1000*20;
			} else if (i==2) {
				baState=BA_JUMP;
				benevolentAiMacroRun("playjump");
				timeout=1000*20;
			}
		} else {
			baState=BA_IDLE;
		}
	} else if (baState==BA_RECHECKFOOD) {
		benevolentAiMacroRun("updvars");
		baState=BA_RECHECKLESSHUNGRY;
	} else if (baState==BA_RECHECKLESSHUNGRY) {
		if (hunger!=oldHunger || happy!=oldHappy) {
			//Okay, we got less hungry/happy. Feed again if needed.
			baState=BA_FEED;
		} else {
			//Hmm, doesn't want to eat. Maybe it's sick?
			benevolentAiMacroRun("medicine");
			baState=BA_FEED;
		}
	} else if (baState==BA_STB) {
		if (lcdmatchMovable(lcd, screen_stb1,-25,0) || lcdmatchMovable(lcd, screen_stb2,-25,0)|| lcdmatchMovable(lcd, screen_stb3,-25,0) || lcdmatchMovable(lcd, screen_stb4,-25,0)) {
			benevolentAiMacroRun("stbshoot");
			timeout=0;
		} else if (lcdmatch(lcd, screen_gameend)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_RECHECKLESSHUNGRY;
		}
	} else if (baState==BA_JUMP) {
		if (lcdmatch(lcd, screen_jump1) || lcdmatch(lcd, screen_jump2)) {
			benevolentAiMacroRun("dojump");
			timeout=0;
		} else if (lcdmatch(lcd, screen_gameend)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_RECHECKLESSHUNGRY;
		}
	}
	return 0;
}
