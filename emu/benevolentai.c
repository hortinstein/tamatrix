#include "benevolentai.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lcdmatch.h"
#include "screens.h"
#include "udp.h"

typedef struct {
	char *name;
	char *code;
} Macro;


static int curMacro=-1;
static int macroPos=0;
static int waitTimeMs=0;
static int cmd, arg;
static int state=0;

static int hunger=-1;
static int happy=-1;
static int oldIcon=-1;
static int iconAttempts=0;

//implementation re https://xkcd.com/534/
long thisAlgorithmBecomingSkynetCost=999999999;

#define ST_IDLE 0
#define ST_NEXT 1
#define ST_ICONSEL 2
#define ST_BTNCHECK 3

#define TAMAUDP_IRTP_CANCEL	0
#define TAMAUDP_IRTP_VISIT	1
#define TAMAUDP_IRTP_GAME	2


static Macro macros[]={
	{"feedmeal", "s2,p2,p2,p2,w90,p3,p3"},
	{"feedsnack", "s2,p2,p1,p2,p2,w90,p3,p3"},
	{"train", "s6,p2,p2,w40"},
	{"medicine", "s7,p2,w40"},
	{"loadeep", "w10,p2,p2,w20"},
	{"updvars", "s1,p2,w10,p1,w10"},
	{"updexit", "p3"},
	{"toilet", "w10,s3,p2,p2,w50"},
	{"toiletpraise", "w10,s3,p2,p2,w50,s6,p2,p1,p2,w50"},
	{"lightoff", "p3,w20"},
	{"playstb", "s4,p2,p2,w90,p2"},
	{"playjump", "s4,p2,p2,w90,p1,p2"},
	{"exitgame", "p3,w90"},
	{"stbshoot", "p2"},
	{"dojump", "p2"},
	{"irgamecl",  "s8,p2,p2,w1,w1,p2"},
	{"irvisitcl", "s8,p2,p2,p1,w1,p2"},
	{"irgamema",  "s8,p2,p2,w1,p2,w1,p2"},
	{"irvisitma", "s8,p2,p2,p1,p2,w1,p2"},
	{"irgamejmp", "p2"},
	{"irfailexit", "p3,w5,p3,w5,p3,w5,p3"},
	{"born", "w100,p3,w20,p3"},
	{"cuddle", "s0,p3,w100"},
	{"tst", "s8"},
	{"", ""}
};



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
	static Display oldLcd;
	static int btnPressedNo;
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
				lcdCopy(&oldLcd, lcd);
				waitTimeMs=400;
				state=ST_BTNCHECK;
				btnPressedNo=arg-1;
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
			} else {
				printf("Huh? Unknown macro cmd %c (macro %d pos %d)\n", cmd, curMacro, macroPos);
				exit(0);
			}
		}
	} else if (state==ST_BTNCHECK) {
		if (!lcdSame(&oldLcd, lcd)) {
			state=ST_NEXT;
		} else {
			//Lcd is the same. Maybe the press didn't take. Just try again once more.
			waitTimeMs=300;
			state=ST_NEXT;
			return (1<<btnPressedNo);
		}
	} else if (state==ST_ICONSEL) {
		iconAttempts++;
		if (iconAttempts==15) state=ST_NEXT; //Bail out.
		if ((arg==0 && lcd->icons==0) || (lcd->icons&(1<<(arg-1)))) {
			state=ST_NEXT;
		} else {
			waitTimeMs=200;
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
#define BA_CHECKFOOD2 2
#define BA_FEED 3
#define BA_RECHECKFOOD 4
#define BA_RECHECKLESSHUNGRY 5
#define BA_RECHECKLESSHUNGRY2 6
#define BA_STB 7
#define BA_JUMP 8
#define BA_IRVISIT 9
#define BA_IRGAME 10


#define CHECKINTERVAL (120*1000)

int baTimeMs=CHECKINTERVAL-5000;
int baState=BA_IDLE;
int oldPxCnt;
int timeout=0;
int irReq=0;
int irMaster=0;


int oldHunger;
int oldHappy;

void benevolentAiDump() {
	char *bastates[]={"idle", "checkfood", "checkfood2", "feed", "recheckfood", "rechecklesshungry", "rechecklesshungry2", "shootthebug", "jump", "irvisit", "irgame"};
	printf("Current macro: ");
	if (state==ST_IDLE) {
		printf("None.\n");
	} else {
		printf("%s, at cmd %c arg %d\n", macros[curMacro].name, cmd, arg);
	}
	printf("Benevolent AI state: %s\n", bastates[baState]);
}


void benevolentAiReqIrComm(int type) {
	irReq=type;
	irMaster=0;
}

void benevolentAiAckIrComm(int type) {
	irReq=type;
	irMaster=1;
}


int benevolentAiDbgCmd(char *cmd) {
	if (strcmp(cmd, "IRG")==0) {
		irReq=TAMAUDP_IRTP_GAME;
		irMaster=1;
		udpSendIrstartReq(irReq);
		baTimeMs=0;
	} else if (strcmp(cmd, "IRV")==0) {
		irReq=TAMAUDP_IRTP_VISIT;
		irMaster=1;
		udpSendIrstartReq(irReq);
		baTimeMs=0;
	} else {
		printf("Commands: irg, irv\n");
	}
	return 1;
}

static int updateHungerHappy(Display *lcd) {
	int i;
	if (!lcdmatch(lcd, screen_hearts)) {
		printf("WtF? Not at hungry/happy screen :/ \n");
		return 0;
	}
	//Assume we're on the hunger/happy screen; we can now measure the amount of heart filled.
	hunger=0; happy=0;
	for (i=0; i<5; i++) {
		if (lcd->p[10][i*10+6]==3) hunger++;
		if (lcd->p[26][i*10+6]==3) happy++;
	}
	return 1;
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
		} else if (lcdmatch(lcd, screen_born1) || lcdmatch(lcd, screen_born2)){
			benevolentAiMacroRun("born");
		} else if (lcdmatchMovable(lcd, screen_sleep1, -16, 2) || lcdmatchMovable(lcd, screen_sleep2, -2, 2)) {
			benevolentAiMacroRun("lightoff");
			baTimeMs=0;
		} else if (getDarkPixelCnt(lcd)>1000) {
			//We turned off the light.
			baTimeMs=0; //Don't wake up to check info
		} else if (lcdmatch(lcd, screen_alert)){
			benevolentAiMacroRun("train");
		} else if (baTimeMs<(CHECKINTERVAL-3000) && ((rand()%300000)<mspassed)) {
			benevolentAiMacroRun("cuddle");
		} else if (baTimeMs<(CHECKINTERVAL-20000) && ((rand()%1000000)<mspassed)) {
			//Invite other tama for a game.
			irReq=TAMAUDP_IRTP_GAME;
			irMaster=1;
			udpSendIrstartReq(irReq);
		} else if (baTimeMs<(CHECKINTERVAL-20000) && ((rand()%1000000)<mspassed)) {
			//Invite other tama for a visit.
			irReq=TAMAUDP_IRTP_VISIT;
			irMaster=1;
			udpSendIrstartReq(irReq);
		} else if (baTimeMs>CHECKINTERVAL || (lcd->icons&(1<<9))) { //check every CHECKINTERVAL ms or if tama wants a attention
			//We need to check for health etc
			baTimeMs=0;
			baState=BA_CHECKFOOD;
		} else if (irReq) {
			//Either we got a request for a visit, or our visit req is ack'ed.
			if (!irMaster) udpSendIrstartAck(irReq);
			if (irReq==TAMAUDP_IRTP_GAME) {
				benevolentAiMacroRun(irMaster?"irgamema":"irgamecl");
				baState=BA_IRGAME;
				timeout=1000*20;
			} else if (irReq==TAMAUDP_IRTP_VISIT) {
				benevolentAiMacroRun(irMaster?"irvisitma":"irvisitcl");
				baState=BA_IRVISIT;
				timeout=1000*20;
			}
			irReq=0;
		} else {
			//Take a snapshot if much changed.
			i=getDarkPixelCnt(lcd);
			if (i<(oldPxCnt-10) || i>(oldPxCnt+10)) {
//				printf("Pix cnt %d, was %d\n", i, oldPxCnt);
				oldPxCnt=(i+oldPxCnt)/2;
				return 8;
			}
		}
	} else if (baState==BA_CHECKFOOD) {
	/*
	Food check routine: (BA_CHECKFOOD is the entry point)
	BA_CHECKFOOD -> updvars -> BA_CHECKFOOD2 -> measure hunger, hearts -> updexit.
	if (food<=4)
		BA_FEED -> feedmeal -> BA_RECHECKFOOD
	else if (happy<5)
		Rand:
			feedsnack -> BA_RECHECKFOOD
			playstb -> BA_STB -> stb etc -> BA_RECHECKFOOD
			playjump -> BA_JUMP -> jump etc -> BA_RECHECKFOOD
			IRTP_GAME -> BA_IDLE -> Done
			IRTP_VISIT -> BA_IDLE -> Done
	BA_RECHECKFOOD -> updvars -> BA_RECHECKLESSHUNGRY -> measure hunger, hearts etc -> updexit -> BA_RECHECKLESSHUNGRY2
	if (hungry==oldhungry && happy==oldhappy)
		BA_CHECKFOOD
	else
		medicine -> BA_CHECKFOOD
	*/
		benevolentAiMacroRun("updvars");
		baState=BA_CHECKFOOD2;
	} else if (baState==BA_CHECKFOOD2) {
		if (updateHungerHappy(lcd)) {
			baState=BA_FEED;
		} else {
			//...We are not at the hearts screen. Weird.
			baState=BA_IDLE;
			baTimeMs=0;
		}
		benevolentAiMacroRun("updexit");
	} else if (baState==BA_FEED) {
		oldHunger=hunger;
		oldHappy=happy;
		if (hunger<4) {
			benevolentAiMacroRun("feedmeal");
			baState=BA_RECHECKFOOD;
		} else if (happy<5) {
			//Bug: If Tama is congested, it doesn't want to go anywhere. Putting it into the feedsnack/playstb/...
			//functions make it do nothing. Timeout will catch this tho'.
			i=rand()%5;
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
			} else if (i==3) {
				//Invite other tama for a game.
				irReq=TAMAUDP_IRTP_GAME;
				irMaster=1;
				udpSendIrstartReq(irReq);
				baTimeMs=0;
				baState=BA_IDLE;
			} else if (i==4) {
				//Invite other tama for a visit.
				irReq=TAMAUDP_IRTP_VISIT;
				irMaster=1;
				udpSendIrstartReq(irReq);
				baTimeMs=0;
				baState=BA_IDLE;
			}
		} else {
			baState=BA_IDLE;
		}
	} else if (baState==BA_RECHECKFOOD) {
		benevolentAiMacroRun("updvars");
		baState=BA_RECHECKLESSHUNGRY;
	} else if (baState==BA_RECHECKLESSHUNGRY) {
		if (updateHungerHappy(lcd)) {
			baState=BA_RECHECKLESSHUNGRY2;
		} else {
			//...We are not at the hearts screen. Weird.
			baState=BA_IDLE;
			baTimeMs=0;
		}
		benevolentAiMacroRun("updexit");
	} else if (baState==BA_RECHECKLESSHUNGRY2) {
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
			baState=BA_RECHECKFOOD;
		} else if (lcdmatch(lcd, screen_doorsel)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_RECHECKFOOD;
		}
	} else if (baState==BA_JUMP) {
		if (lcdmatch(lcd, screen_jump1) || lcdmatch(lcd, screen_jump2)) {
			benevolentAiMacroRun("dojump");
			timeout=0;
		} else if (lcdmatch(lcd, screen_gameend)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_RECHECKFOOD;
		} else if (lcdmatch(lcd, screen_doorsel)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_RECHECKFOOD;
		}
	} else if (baState==BA_IRVISIT) {
		//Erm... Just wait till timeout is done.
		if (lcdmatch(lcd, screen_irfail)) {
			benevolentAiMacroRun("irfailexit");
			baState=BA_IDLE;
		}
	} else if (baState==BA_IRGAME) {
		if (lcdmatch(lcd, screen_irgame1)) {
			benevolentAiMacroRun("irgamejmp");
//			timeout=0;
		} else if (lcdmatch(lcd, screen_irfail)) {
			benevolentAiMacroRun("irfailexit");
			baState=BA_IDLE;
		} else if (lcdmatch(lcd, screen_gameend)) {
			benevolentAiMacroRun("exitgame");
			baState=BA_IDLE;
		}
	}
	return 0;
}
