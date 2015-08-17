#include "benevolentai.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	char *name;
	char *code;
} Macro;


int curMacro=-1;
int macroPos=0;
int waitTimeMs=0;
int cmd, arg;
int state=0;


#define ST_IDLE 0
#define ST_NEXT 1
#define ST_ICONSEL 2



static Macro macros[]={
	{"feedmeal", "s2,p2,p2,p2,w80,p3"},
	{"feedsnack", "s2,p2,p1,p2,p2,w80,p3"},
	{"loadeep", "w10,p2,p2"},
	{"tst", "s8"},
	{"", ""}
};



int benevolentAiMacroRun(char *name) {
	int i=0;
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
			while (macros[curMacro].code[macroPos]!=',' && macros[curMacro].code[macroPos]!=0) macroPos++;
			if (macros[curMacro].code[macroPos]==',') macroPos++;
			if (cmd=='p') {
				//Press a button
				waitTimeMs=200;
				return (1<<(arg-1));
			} else if (cmd=='w') {
				//Wait x deciseconds
				waitTimeMs=arg*100;
				return 0;
			} else if (cmd=='s') {
				//Select icon...
				state=ST_ICONSEL;
				waitTimeMs=0;
				return 0;
			} else {
				printf("Huh? Unknown macro cmd %c (macro %d pos %d)\n", cmd, curMacro, macroPos);
				exit(0);
			}
		}
	} else if (state==ST_ICONSEL) {
		if (lcd->icons&(1<<(arg-1))) {
			state=ST_NEXT;
		} else {
			waitTimeMs=300;
			return (1<<0);
		}
	}
	return 0;
}


int benevolentAiRun(Display *lcd, int mspassed) {
	int r=macroRun(lcd, mspassed);
	if (r<0) return 0;
	return r;
}
