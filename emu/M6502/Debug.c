/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                          Debug.c                        **/
/**                                                         **/
/** This file contains the built-in debugging routine for   **/
/** the 6502 emulator which is called on each 6502 step     **/
/** when Trap!=0. Some NES-dependent features are also      **/
/** included under #ifdef INES.                             **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2007                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/
#include "M6502.h"
#ifdef DEBUG

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../tamaemu.h"
#include "../benevolentai.h"
#include "../lcd.h"

#ifdef INES
#include "NES.h"
extern word VAddr;
extern byte CURLINE;
#endif

#define RDWORD(A) (R->Rd6502(R, A+1)*256+R->Rd6502(R, A))

enum AddressingModes { Ac=0,Il,Im,Ab,Zp,Zx,Zy,Ax,Ay,Rl,Ix,Iy,In,No };

static const byte *Ops[]=
{
  "adc","and","asl","bcc","bcs","beq","bit","bmi",
  "bne","bpl","brk","bvc","bvs","clc","cld","cli", 
  "clv","cmp","cpx","cpy","dec","dex","dey","inx",
  "iny","eor","inc","jmp","jsr","lda","nop","ldx",
  "ldy","lsr","ora","pha","php","pla","plp","rol",
  "ror","rti","rts","sbc","sta","stx","sty","sec",
  "sed","sei","tax","tay","txa","tya","tsx","txs"
};

static const byte Ads[512]=
{
  10,Il, 34,Ix, No,No, No,No, No,No, 34,Zp,  2,Zp, No,No,
  36,Il, 34,Im,  2,Ac, No,No, No,No, 34,Ab,  2,Ab, No,No,
   9,Rl, 34,Iy, No,No, No,No, No,No, 34,Zx,  2,Zx, No,No,
  13,Il, 34,Ay, No,No, No,No, No,No, 34,Ax,  2,Ax, No,No,
  28,Ab,  1,Ix, No,No, No,No,  6,Zp,  1,Zp, 39,Zp, No,No,
  38,Il,  1,Im, 39,Ac, No,No,  6,Ab,  1,Ab, 39,Ab, No,No,
   7,Rl,  1,Iy, No,No, No,No, No,No,  1,Zx, 39,Zx, No,No,
  47,Il,  1,Ay, No,No, No,No, No,No,  1,Ax, 39,Ax, No,No,
  41,Il, 25,Ix, No,No, No,No, No,No, 25,Zp, 33,Zp, No,No,
  35,Il, 25,Im, 33,Ac, No,No, 27,Ab, 25,Ab, 33,Ab, No,No,
  11,Rl, 25,Iy, No,No, No,No, No,No, 25,Zx, 33,Zx, No,No,
  15,Il, 25,Ay, No,No, No,No, No,No, 25,Ax, 33,Ax, No,No,
  42,Il,  0,Ix, No,No, No,No, No,No,  0,Zp, 40,Zp, No,No,
  37,Il,  0,Im, 40,Ac, No,No, 27,In,  0,Ab, 40,Ab, No,No,
  12,Rl,  0,Iy, No,No, No,No, No,No,  0,Zx, 40,Zx, No,No,
  49,Il,  0,Ay, No,No, No,No, No,No,  0,Ax, 40,Ax, No,No,
  No,No, 44,Ix, No,No, No,No, 46,Zp, 44,Zp, 45,Zp, No,No,
  22,Il, No,No, 52,Il, No,No, 46,Ab, 44,Ab, 45,Ab, No,No,
   3,Rl, 44,Iy, No,No, No,No, 46,Zx, 44,Zx, 45,Zy, No,No,
  53,Il, 44,Ay, 55,Il, No,No, No,No, 44,Ax, No,No, No,No,
  32,Im, 29,Ix, 31,Im, No,No, 32,Zp, 29,Zp, 31,Zp, No,No,
  51,Il, 29,Im, 50,Il, No,No, 32,Ab, 29,Ab, 31,Ab, No,No,
   4,Rl, 29,Iy, No,No, No,No, 32,Zx, 29,Zx, 31,Zy, No,No,
  16,Il, 29,Ay, 54,Il, No,No, 32,Ax, 29,Ax, 31,Ay, No,No,
  19,Im, 17,Ix, No,No, No,No, 19,Zp, 17,Zp, 20,Zp, No,No,
  24,Il, 17,Im, 21,Il, No,No, 19,Ab, 17,Ab, 20,Ab, No,No,
   8,Rl, 17,Iy, No,No, No,No, No,No, 17,Zx, 20,Zx, No,No,
  14,Il, 17,Ay, No,No, No,No, No,No, 17,Ax, 20,Ax, No,No,
  18,Im, 43,Ix, No,No, No,No, 18,Zp, 43,Zp, 26,Zp, No,No,
  23,Il, 43,Im, 30,Il, No,No, 18,Ab, 43,Ab, 26,Ab, No,No,
   5,Rl, 43,Iy, No,No, No,No, No,No, 43,Zx, 26,Zx, No,No,
  48,Il, 43,Ay, No,No, No,No, No,No, 43,Ax, 26,Ax, No,No
};

/** DAsm() ****************************************************/
/** This function will disassemble a single command and      **/
/** return the number of bytes disassembled.                 **/
/**************************************************************/
static int DAsm(M6502 *R, char *S,word A)
{
  byte J;
  word B,OP,TO;

  B=A;OP=R->Rd6502(R, B++)*2;

  switch(Ads[OP+1])
  {
    case Ac: sprintf(S,"%s a",Ops[Ads[OP]]);break;
    case Il: sprintf(S,"%s",Ops[Ads[OP]]);break;

    case Rl: J=R->Rd6502(R, B++);TO=A+2+((J<0x80)? J:(J-256)); 
             sprintf(S,"%s $%04X",Ops[Ads[OP]],TO);break;

    case Im: sprintf(S,"%s #$%02X",Ops[Ads[OP]],R->Rd6502(R, B++));break;
    case Zp: sprintf(S,"%s $%02X",Ops[Ads[OP]],R->Rd6502(R, B++));break;
    case Zx: sprintf(S,"%s $%02X,x",Ops[Ads[OP]],R->Rd6502(R, B++));break;
    case Zy: sprintf(S,"%s $%02X,y",Ops[Ads[OP]],R->Rd6502(R, B++));break;
    case Ix: sprintf(S,"%s ($%02X,x)",Ops[Ads[OP]],R->Rd6502(R, B++));break;
    case Iy: sprintf(S,"%s ($%02X),y",Ops[Ads[OP]],R->Rd6502(R, B++));break;

    case Ab: sprintf(S,"%s $%04X",Ops[Ads[OP]],RDWORD(B));B+=2;break;
    case Ax: sprintf(S,"%s $%04X,x",Ops[Ads[OP]],RDWORD(B));B+=2;break;
    case Ay: sprintf(S,"%s $%04X,y",Ops[Ads[OP]],RDWORD(B));B+=2;break;
    case In: sprintf(S,"%s ($%04X)",Ops[Ads[OP]],RDWORD(B));B+=2;break;

    default: sprintf(S,".db $%02X",OP/2);
  }
  return(B-A);
}

/** Debug6502() **********************************************/
/** This function should exist if DEBUG is #defined. When   **/
/** Trace!=0, it is called after each command executed by   **/
/** the CPU, and given the 6502 registers. Emulation exits  **/
/** if Debug6502() returns 0.                               **/
/*************************************************************/
byte Debug6502(M6502 *R)
{
  static char FA[9]="NVRBDIZC";
  char S[128];
  byte *P,F;
  int J,I,K;
		Tamagotchi *tama=(Tamagotchi *)R->User;

  DAsm(R, S,R->PC.W);

  printf
  (
    "A:%02X  P:%02X  X:%02X  Y:%02X  S:%04X  PC:%04X  Flags:[",
    R->A,R->P,R->X,R->Y,R->S+0x0100,R->PC.W
  );

  for(J=0,F=R->P;J<8;J++,F<<=1)
    printf("%c",F&0x80? FA[J]:'.');
  printf("]\n");

  printf
  (
    "AT PC: [%02X - %s]   AT SP: [%02X %02X %02X]\n",
    R->Rd6502(R, R->PC.W),S,
    R->Rd6502(R, 0x0100+(byte)(R->S+1)),
    R->Rd6502(R, 0x0100+(byte)(R->S+2)),
    R->Rd6502(R, 0x0100+(byte)(R->S+3))
  );

  while(1)
  {
    printf("\n[Command,'?']-> ");
    fflush(stdout);fflush(stdin);

    fgets(S,50,stdin);
    for(J=0;S[J]>=' ';J++)
      S[J]=toupper(S[J]);
    S[J]='\0';

    switch(S[0])
    {
      case 'H':
      case '?':
        printf("\n***** Built-in 6502 Debugger Commands *****\n");
        printf("<CR>       : Break at the next instruction\n");
        printf("= <addr>   : Break at addr\n");
        printf("+ <offset> : Break at PC + offset\n");
        printf("c          : Continue without break\n");
        printf("j <addr>   : Continue from addr\n");
        printf("m <addr>   : Memory dump at addr\n");
        printf("d <addr>   : Disassembly at addr\n");
        printf("v          : Show interrupt vectors\n");
#ifdef INES
        printf("p          : Show NES palette\n");
        printf("r          : Show NES hardware registers\n");
        printf("s          : Show sprite attributes\n");
        printf("s <number> : Show sprite pattern\n");
#endif /* INES */
		printf("r          : Show hardware reg info\n");
		printf("u          : Toggle break on unknown io r/w\n");
		printf("p x        : Press button x\n");
		printf("e macroname: Execute given macro name\n");
		printf("l filename : Dump lcd data to file\n");
        printf("?,h        : Show this help text\n");
        printf("q          : Exit emulation\n");
        break;

      case '\0': return(1);
      case '=':  if(strlen(S)>=2)
                 { sscanf(S+1,"%hX",&(R->Trap));R->Trace=0;return(1); }
                 break;
      case '+':  if(strlen(S)>=2)
                 {
                   sscanf(S+1,"%hX",&(R->Trap));
                   R->Trap+=R->PC.W;R->Trace=0;
                   return(1);
                 }
                 break;
      case 'J':  if(strlen(S)>=2)
                 { sscanf(S+1,"%hX",&(R->PC.W));R->Trace=0;return(1); }
                 break;
      case 'C':  R->Trap=0xFFFF;R->Trace=0;return(1); 
      case 'Q':  return(0);
      case 'U':
		tamaToggleBkunk((Tamagotchi *)R->User);
		break;
	  case 'P':
		tamaPressBtn((Tamagotchi*)R->User, atoi(S+1));
		R->Trace=0;
		return(1);
      case 'L':
		lcdDump(tama->dram, tama->lcd.sizex, tama->lcd.sizey, S+2);
		R->Trace=0;
		return 1;
      case 'V':
        printf("\n6502 Interrupt Vectors:\n");
        printf("[$FFFC] INIT: $%04X\n",R->Rd6502(R, 0xFFFC)+256*R->Rd6502(R, 0xFFFD));
        printf("[$FFFE] IRQ:  $%04X\n",R->Rd6502(R, 0xFFFE)+256*R->Rd6502(R, 0xFFFF));
        printf("[$FFFA] NMI:  $%04X\n",R->Rd6502(R, 0xFFFA)+256*R->Rd6502(R, 0xFFFB));
        break;

      case 'M':
        {
          word Addr;

          if(sscanf(S+1,"%hX",&Addr)!=1) Addr=R->PC.W;
          printf("\n");
          for(J=0;J<16;J++)
          {
            printf("%04X: ",Addr);
            for(I=0;I<16;I++,Addr++)
              printf("%02X ",R->Rd6502(R, Addr));
            printf(" | ");Addr-=16;
            for(I=0;I<16;I++,Addr++)
              printf("%c",isprint(R->Rd6502(R, Addr))? R->Rd6502(R, Addr):'.');
            printf("\n");
          }
        }
        break;

      case 'D':
        {
          word Addr;

          if(sscanf(S+1,"%hX",&Addr)!=1) Addr=R->PC.W;
          printf("\n");
          for(J=0;J<16;J++)
          {
            printf("%04X: ",Addr);
            Addr+=DAsm(R, S,Addr);
            printf("%s\n",S);
          }
        }
        break;

      case 'R':
        printf("Hw dump:\n");
        tamaDumpHw(R);
        break;
      case 'E':
        if (benevolentAiMacroRun(S+2)) {
			R->Trace=0;
			return 1;
		}
        break;
    }
  }

  /* Continue with emulation */  
  return(1);
}

#endif /* DEBUG */
