
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <esp8266.h>
#include <../esphttpclient/httpclient.h>
#include "img.h"

static ETSTimer checkBtntimer;
static ETSTimer btnOffTimer;
static ETSTimer macroTimer;
static ETSTimer reqTimer;


static void ICACHE_FLASH_ATTR reqTimerCb(void *arg);

#define  BTNGPIO 0

#define TAMA_B1			0 //d6
#define TAMA_B2			13 //d7
#define TAMA_B3			12 //d8

#define TAMAOUT		((1<<0)|(1<<13)|(1<<12))


typedef struct  {
	char button;
	int delay;
} Macro;

Macro macro[]={
	{1,3000},
	{2, 1000},	//menu
	{2, 6000}, //download
	{1, 500},	////go to figure
	{1, 500},
	{1, 500},
	{1, 500},
	{1, 500},	
	{2, 500},	//enter figure menu
	{2, 24000},	//game
	{2, 500},	//pick 1st game
	{2, 500},	//partner
	{2, 3000},  //whatever
	{0,0}		//done
};

int macroPos=0;

int tamaToShow=0;
int dontShowIncomingImage=0;

static void ICACHE_FLASH_ATTR btnOffTimerCb(void *arg) {
	gpio_output_set(TAMAOUT, 0, 0, TAMAOUT);
}

static void ICACHE_FLASH_ATTR macroTimerCb(void *arg) {
	if (macro[macroPos].button==0) {
		os_timer_disarm(&reqTimer);
		os_timer_setfn(&reqTimer, reqTimerCb, NULL);
		os_timer_arm(&reqTimer, 2000, 0);
	os_timer_arm(&checkBtntimer, 50, 1);
		return;
	}

	os_printf("Macro: Send button %d delay %dms\n", macro[macroPos].button, macro[macroPos].delay);
	if (macro[macroPos].button==1) {
		gpio_output_set(0, 1<<TAMA_B1, 1<<TAMA_B1, 0);
	} else if (macro[macroPos].button==2) {
		gpio_output_set(0, 1<<TAMA_B2, 1<<TAMA_B2, 0);
	} else if (macro[macroPos].button==3) {
		gpio_output_set(0, 1<<TAMA_B3, 1<<TAMA_B3, 0);
	}

	os_timer_disarm(&btnOffTimer);
	os_timer_setfn(&btnOffTimer, btnOffTimerCb, NULL);
	os_timer_arm(&btnOffTimer, 200, 0);


	os_timer_disarm(&macroTimer);
	os_timer_arm(&macroTimer, macro[macroPos].delay, 0);
	macroPos++;
}



void lcdSendByte(char b) {
	int x;

	for (x=0; x<8; x++) {
		if (b&0x80) {
			gpio_output_set((1<<TAMA_B1), 0, 0, 0);
		} else {
			gpio_output_set(0, (1<<TAMA_B1), 0, 0);
		}
		if (x&1) {
			gpio_output_set((1<<TAMA_B2), 0, 0, 0);
		} else {
			gpio_output_set(0, (1<<TAMA_B2), 0, 0);
		}
		b<<=1;
		os_delay_us(38);
	}
}



void datacb(char * resp, int http_status, char * fullresp) {
	int x;
	if (dontShowIncomingImage) return;

	os_timer_disarm(&reqTimer);
	os_timer_arm(&reqTimer, 100, 0);
	if (http_status == HTTP_STATUS_GENERIC_ERROR) return;
	os_printf("Got img.\n");

	gpio_output_set(TAMAOUT, 0, TAMAOUT, 0);
	gpio_output_set(0, (1<<TAMA_B3), 0, 0);
	os_delay_us(20);
	for (x=0; x<(48*32/4); x++) lcdSendByte(resp[x]);
	gpio_output_set((1<<TAMA_B3), 0, 0, 0);
	lcdSendByte(0xff);
	gpio_output_set(TAMAOUT, 0, 0, TAMAOUT);
}



static void ICACHE_FLASH_ATTR reqTimerCb(void *arg) {
	char buff[256];
	dontShowIncomingImage=0;
	os_sprintf(buff, "http://tamahive.spritesserver.nl/gettamabin-real.php?tama=%d" , tamaToShow);
	http_get(buff, "", datacb);
	os_timer_arm(&reqTimer, 1000, 0);
}


static void ICACHE_FLASH_ATTR showTamaNo() {
	int x, y;
	int d1, d2;
	d1=(tamaToShow/10);
	d2=(tamaToShow%10);
	os_timer_disarm(&reqTimer);
	dontShowIncomingImage=1;

	gpio_output_set(TAMAOUT, 0, TAMAOUT, 0);
	gpio_output_set(0, (1<<TAMA_B3), 0, 0);
	os_delay_us(20);
	for (y=0; y<32; y++) {
		for (x=0; x<48/4; x++) {
			if (x<6) {
				lcdSendByte(imgNo[d1][(y*6)+x]);
			} else {
				lcdSendByte(imgNo[d2][(y*6)+x-6]);
			}
		}
	}
	gpio_output_set((1<<TAMA_B3), 0, 0, 0);
	lcdSendByte(0xff);
	gpio_output_set(TAMAOUT, 0, 0, TAMAOUT);

	os_timer_arm(&reqTimer, 2000, 0);
}

static void ICACHE_FLASH_ATTR handleButton(int button) {
	if (button==(1<<TAMA_B1)) {
		tamaToShow--;
		showTamaNo();
	}
	if (button==(1<<TAMA_B3)) {
		tamaToShow++;
		showTamaNo();
	}
}

static void ICACHE_FLASH_ATTR checkBtnTimerCb(void *arg) {
	static int resetCnt=0;
	static int oldBtn=0;
	int t;
	int btns=gpio_input_get()&TAMAOUT;
	
	//Extract buttons that were released
	t=(btns^oldBtn)&oldBtn;
	if (t!=0) handleButton(t);
	oldBtn=btns;

	//Handle reset button if pressed >1s
	if (!(btns&(1<<0))) {
		resetCnt++;
	} else {
		if (resetCnt>=60) { //3 sec pressed
			wifi_station_disconnect();
			wifi_set_opmode(0x3); //reset to AP+STA mode
			os_printf("Reset to AP mode. Restarting system...\n");
			system_restart();
		}
		resetCnt=0;
	}
}


void ioInit() {
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

	os_timer_disarm(&checkBtntimer);
	os_timer_setfn(&checkBtntimer, checkBtnTimerCb, NULL);


	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	gpio_output_set(TAMAOUT, 0, 0, TAMAOUT);

	os_timer_disarm(&macroTimer);
	os_timer_setfn(&macroTimer, macroTimerCb, NULL);
	os_timer_arm(&macroTimer, 100, 0);
}

