#ifndef TAMAEMU_H
#define TAMAEMU_H

#define FCPU 16000000

#include <stdint.h>
#include "M6502/M6502.h"
#include "i2c.h"
#include "i2ceeprom.h"

#define R_BANK		0x3000
#define R_CLKCTL	0x3001
#define R_C32KCTL	0x3002
#define R_WDTCTL	0x3004
#define R_RESFL		0x3005
#define R_SPUCTL	0x3006
#define R_WAKEEN	0x3007
#define R_WAKEFL	0x3008
#define R_RESENA	0x300B
#define R_RESCTL	0x300C
#define R_PACFG		0x3010
#define R_PADIR		0x3011
#define R_PADATA	0x3012
#define R_PASTR		0x3013
#define R_PBCFG		0x3014
#define R_PBDIR		0x3015
#define R_PBDATA	0x3016
#define R_PCCFG		0x3017
#define R_PCDIR		0x3018
#define R_PCDATA	0x3019
#define R_TIMBASE	0x3030
#define R_TIMCTL	0x3031
#define R_TM0LO		0x3032
#define R_TM0HI		0x3033
#define R_TM1LO		0x3034
#define R_TM1HI		0x3035
#define R_CARCTL	0x303C
#define R_KEYSCTL	0x303D
#define R_KEYSP1	0x303E
#define R_KEYSP2	0x303F
#define R_LCDS1		0x3040
#define R_LCDS2		0x3041
#define R_LCDC1		0x3042
#define R_LCDC2		0x3043
#define R_LCDSEG	0x3044
#define R_LCDCOM	0x3045
#define R_LCDFRMCTL	0x3046
#define R_LCDBUFCTL	0x3047
#define R_VLCDCTL	0x3048
#define R_PUMPCTL	0x3049
#define R_BIASCTL	0x304A
#define R_MVCL		0x3051
#define R_MIXR		0x3052
#define R_MIXL		0x3053
#define R_SPUEN		0x3054
#define R_SPUINTSTS	0x3055
#define R_SPUINTEN	0x3056
#define R_CONC		0x3057
#define R_MULA		0x3058
#define R_MULB		0x3059
#define R_MULOUTH	0x305A
#define R_MULOUTL	0x305B
#define R_MULACT	0x305C
#define R_AUTOMUTE	0x305E
#define R_DAC1H		0x3060
#define R_DAC1L		0x3062
#define R_SOFTCTL	0x3064
#define R_DACPWM	0x3065
#define R_INTCTLLO	0x3070
#define R_INTCTLMI	0x3071
#define R_INTCLRLO	0x3073
#define R_INTCLRMI	0x3074
#define R_NMICTL	0x3076
#define R_LVCTL		0x3077
#define R_DATAX		0x3080
#define R_DATAY		0x3081
#define R_DATA0XH	0x3082
#define R_DATA0XL	0x3083
#define R_DATA0YH	0x3084
#define R_DATA0YL	0x3085
#define R_DATAXH0	0x3086
#define R_DATAXL0	0x3087
#define R_DATAYH0	0x3088
#define R_DATAYL0	0x3089
#define R_DATAXLXH	0x308A
#define R_DATAYLYH	0x308B
#define R_DATAXLYH	0x308C
#define R_DATAYLXH	0x308D
#define R_IFFPCLR	0x3090
#define R_IF8KCLR	0x3093
#define R_IF2KCLR	0x3094
#define R_IFTM0CLR	0x3097
#define R_IFTBLCLR	0x309A
#define R_IFTBHCLR	0x309B
#define R_IFTM1CLR	0x309D
#define R_SPICTL	0x30B0
#define R_SPITXSTS	0x30B1
#define R_SPITXCTL	0x30B2
#define R_SPITXDAT	0x30B3
#define R_SPIRXSTS	0x30B4
#define R_SPIRXCTL	0x30B5
#define R_SPIRXDAT	0x30B6
#define R_SPIMISC	0x30B7
#define R_SPIPORT	0x30BA

#define IRQVECT_T0			0xFFC0
#define IRQVECT_FROSCD2K	0xFFC6
#define IRQVECT_FROSCD8K	0xFFC8
#define IRQVECT_SPU			0xFFCA
#define IRQVECT_SPI			0xFFCC
#define IRQVECT_FP			0xFFCE
#define IRQVECT_T1			0xFFD4
#define IRQVECT_TBH			0xFFD8
#define IRQVECT_TBL			0xFFDA
#define IRQVECT_NMI			0xFFFA

typedef struct {
	int tblDiv, tblCtr;
	int tbhDiv, tbhCtr;
	int c8kCtr, c2kCtr;
	int t0Div, t0Ctr;
	int t1Div, t1Ctr;
	int fpCtr;
	int cpuDiv, cpuCtr;
} TamaClk;

typedef struct {
	uint8_t bankSel;
	uint8_t portAdata;
	uint8_t portBdata;
	uint8_t portCdata;
	uint8_t portAout;
	uint8_t portBout;
	uint8_t portCout;
	int32_t ticks;
	int16_t iflags;
	int8_t nmiflags;
	int remCpuCycles;
	int lastInt;
} TamaHw;


typedef struct {
	int sizex;
	int sizey;
} TamaLcd;

typedef struct {
	M6502 *cpu;
	unsigned char **rom;
	I2cBus *i2cbus;
	I2cEeprom *i2ceeprom;
	char ram[1536];
	char dram[512];
	char ioreg[255];
	TamaHw hw;
	TamaLcd lcd;
	TamaClk clk;
	int bkUnk;
	int btnPressed;
	int btnReleaseTm;
	int btnReads;
	int irnx;
} Tamagotchi;


unsigned char **loadRoms();
void freeRoms(unsigned char **roms);
Tamagotchi *tamaInit(unsigned char **rom, char *eepromFile);
void tamaRun(Tamagotchi *tama, int cycles);
void tamaToggleBkunk(Tamagotchi *t);
void tamaPressBtn(Tamagotchi*t, int btn);


#endif