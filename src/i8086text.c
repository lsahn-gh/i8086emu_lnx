/* i8086emu
 * Copyright (C) 2004 Joerg Mueller-Hipper, Robert Dinse, Fred Brodmueller, Christian Steineck
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
 
/****************************************************/
/* Autor: Gemischt                                  */
/* Version: 1.0                                     */
/* ** => nicht beendet                              */
/*                                                  */
/*                                                  */
/* Vorsicht:                                        */
/* dieser Source ist schlecht strukturiert und      */
/* kommentiert					                            */
/* war ursprünglich nur zu Testzwecken gedacht      */
/*                                                  */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>
#include <string.h>
#include "i8086proc.h"
#include "i8086error.h"
#include "i8086datatrans.h"
#include "i8086messages.h"
#include "i8086config.h"
#include "i8086util.h"
#include "i8086emufuncs.h"
#include "disasm/disasm.h"


#define HIDE_CODE_VIEWER 1
#define LOAD_ROM 2

#define keypressclocks 1900;

 char testkey=0;
 unsigned char prellbock=0;
 unsigned int keydelay=0;
 i808616BitAdr breakpoint=0;

 i808616BitAdr stepover=0;

 i8086msgFunc *oldPortHandler=NULL;


void clearCmd();

typedef struct codeView
{
  unsigned short startAdr;
  unsigned short endAdr;
}codeView;

WINDOW *regWin, *cmdWin, *memWin, *flagWin, *ledWin, *switchWin, *codeWin;
unsigned char args=0; /* gesetzte Programmargumente */

void printHelp()
{
  char c;

  mvwprintw(cmdWin, 1, 1, "m - Print StrAdr. q - Quit. r - Set Reg. w - Set Mem");
  wrefresh(cmdWin);
  wscanw(cmdWin, "%c", &c);
  clearCmd();
}

void printReg()
{
  i8086SingleRegister *reg;

  reg = i8086GetRegister(core, 1, i8086_REG_AX);
  mvwprintw(regWin, 1, 1, "AX %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_BX);
  mvwprintw(regWin, 1, 9, "BX %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_CX);
  mvwprintw(regWin, 2, 1, "CX %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_DX);
  mvwprintw(regWin, 2, 9, "DX %04x", reg->x);
  free(reg);

  reg = i8086GetRegister(core, 1, i8086_REG_SP);
  mvwprintw(regWin, 3, 9, "SP %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_BP);
  mvwprintw(regWin, 1, 17, "BP %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_SI);
  mvwprintw(regWin, 2, 17, "SI %04x", reg->x);
  free(reg);
  reg = i8086GetRegister(core, 1, i8086_REG_DI);
  mvwprintw(regWin, 3, 17, "DI %04x", reg->x);
  free(reg);

  reg = i8086GetSegRegister(core,i8086_REG_CS, 0);
  mvwprintw(regWin, 1, 27, "CS %04x", reg->x);
  free(reg);
  reg = i8086GetSegRegister(core, i8086_REG_DS, 0);
  mvwprintw(regWin, 2, 27, "DS %04x", reg->x);
  free(reg);
  reg = i8086GetSegRegister(core, i8086_REG_SS, 0);
  mvwprintw(regWin, 3, 27, "SS %04x", reg->x);
  free(reg);
  reg = i8086GetSegRegister(core, i8086_REG_ES, 0);
  mvwprintw(regWin, 1, 35, "ES %04x", reg->x);
  free(reg);

  mvwprintw(regWin, 3, 1, "PC %04x", core->pc);

  wrefresh(regWin);
}

void printFlags()
{
  if ((core->flags&i8086_FLAG_CF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 1, 1, "CF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_PF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 2, 1, "PF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_ACF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 3, 1, "ACF");
  wattroff(flagWin, A_STANDOUT);

  if ((core->flags&i8086_FLAG_ZF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 1, 5, "ZF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_SF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 2, 5, "SF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_TF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 3, 5, "TF");
  wattroff(flagWin, A_STANDOUT);

  if ((core->flags&i8086_FLAG_IF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 1, 9, "IF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_DF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 2, 9, "DF");
  wattroff(flagWin, A_STANDOUT);
  if ((core->flags&i8086_FLAG_OF)>0)
    wattron(flagWin, A_STANDOUT);
  mvwprintw(flagWin, 3, 9, "OF");
  wattroff(flagWin, A_STANDOUT);

  wrefresh(flagWin);
}

void printLeds(int signo)
{
  unsigned char i,j;
  char leds=portOpByte(core,i8086_OUTPUT_PORT,0,0,i8086_READ_PORT);
  j=128;
  for (i=7;i!=255;i--)
  {
    if ((leds&j)>0)
        wattron(ledWin, A_STANDOUT);
    mvwprintw(ledWin, 1, (7-i)*2+3,"%i",i);
    wattroff(ledWin, A_STANDOUT);
    j=j/2;
  }

  wrefresh(ledWin);
}

void printSwitches()
{
  unsigned char i,j;
  //char leds=portReadByte(core,0);
  //char switches=core->ports.x[0]; //leseport 0 entspricht i8086_PORTS
  char switches=portOpByte(core,i8086_INPUT_PORT,0,0,i8086_READ_PORT);
  j=128;
  for (i=7;i!=255;i--)
  {
    if ((switches&j)>0)
        wattron(switchWin, A_STANDOUT);
    mvwprintw(switchWin, 1, (7-i)*2+3,"%i",i);
    wattroff(switchWin, A_STANDOUT);
    j=j/2;
  }

  wrefresh(switchWin);
}

void printDisplay()
{

  unsigned char i,row=23,line=0;
  unsigned char d=255;

  for (i=0;i<=7;i++)
  {

    d=portOpByte(core,i8086_OUTPUT_PORT,0,158-(i*2),i8086_READ_PORT);
    if (d&1) mvwprintw(cmdWin,line,(row+1)+i*4,"_");
  else mvwprintw(cmdWin,line,(row+1)+i*4," ");
    if (d&2) mvwprintw(cmdWin,line+1,(row+2)+i*4,"|");
  else mvwprintw(cmdWin,line+1,(row+2)+i*4," ");
    if (d&4) mvwprintw(cmdWin,line+2,(row+2)+i*4,"|");
  else mvwprintw(cmdWin,line+2,(row+2)+i*4," ");
    if (d&8) mvwprintw(cmdWin,line+2,(row+1)+i*4,"_");
  else mvwprintw(cmdWin,line+2,(row+1)+i*4," ");
    if (d&16) mvwprintw(cmdWin,line+2,(row)+i*4,"|");
  else mvwprintw(cmdWin,line+2,(row)+i*4," ");
    if (d&32) mvwprintw(cmdWin,line+1,(row)+i*4,"|");
  else mvwprintw(cmdWin,line+1,(row)+i*4," ");
    if (d&64) mvwprintw(cmdWin,line+1,(row+1)+i*4,"_");
  else mvwprintw(cmdWin,line+1,(row+1)+i*4," ");
    if (d&128) mvwprintw(cmdWin,line+2,(row+3)+i*4,".");
  else mvwprintw(cmdWin,line+2,(row+3)+i*4," ");
 }

  wrefresh(cmdWin);
}

/* Gibt den Speicher ab Adresse startAdr aus */
void printMem(unsigned short startAdr)
{
  unsigned short i=0, j=0;

  for (j=0; j<16; j++)
    mvwprintw(memWin, 1, 6+(j*3), "%02x", j);
  for (i=0; i<12; i++)
  {
    mvwprintw(memWin, i+2, 1, "%04x", (unsigned short)(startAdr+(i*16)));
    for (j=0; j<16; j++)
    {
      if (core->pc==j+startAdr+(i*16) + i8086GetSegRegister_fast(core, i8086_REG_CS, 1))
        wattron(memWin, A_STANDOUT); //kleine erweiterung um den pc im speicherfenster zu markieren (Perrin)
      mvwprintw(memWin, i+2, 6+(j*3), "%02x", core->mem[(unsigned short)(startAdr+(i*16)+j)]);
      wattroff(memWin, A_STANDOUT);
     }
  }
  wrefresh(memWin);
}

/* Gibt den disassamblierten Code im codeWin aus. */
/* cv enthaelt interne Steuerdaten fuer die       */
/* Pruefung ob das Viewersegment neu gesetzt      */
/* werden muss.                                   */
/* cmds gibt das Array mit den Emulatorfkt. an.   */
/* Return: i8086_SUC_EXECCMD -> Erfolg,           */
/*         sonst i8086_ERR_ILGOPCODE              */
int printCode(i8086core *core, i8086command **cmds, codeView *cv)
{
  unsigned char opcode, i, pos, read=0;
  //unsigned char dispLn=0;
  //i8086Parameter para, data;
  //unsigned char str[6];
  //unsigned short ret=0;
  unsigned short startAdr;
  unsigned short cs;
  unsigned char *cmddata;
  char *output;
  unsigned char cmdsize;

   if (args & HIDE_CODE_VIEWER) /* pruefen ob Codeviewer aktiviert ist */
    return i8086_SUC_EXECCMD;

  cs = i8086GetSegRegister_fast(core, i8086_REG_CS, 1);

  if ( (core->pc+cs > cv->endAdr) || (core->pc+cs < cv->startAdr) ) /* Viewersegment setzen */
    startAdr = core->pc+cs;                                 /* pc ist ueber bzw. unter Windowgrenzen gekommen */
  else
    startAdr = cv->startAdr;

  for (pos=0; pos<16; pos++)
  {
    opcode = core->mem[startAdr+read];         /* opcode lesen     */

    if (core->pc+cs==startAdr+read)               /* aktuellen OpCode sichtbar machen */
    {
      wattron(codeWin, A_STANDOUT);
    }

    if (startAdr+read==breakpoint)
    mvwprintw(codeWin, 1+pos, 1, ".");
    else
    mvwprintw(codeWin, 1+pos, 1, " ");
    mvwprintw(codeWin, 1+pos, 2, "%0004x", startAdr+read); /* Adressen ausgeben */
    wclrtoeol(codeWin);
    wattroff(codeWin, A_STANDOUT);

    if (cmds[opcode]==NULL)                    /* illegaler Opcode */
    {
      int j;
      //sprintf(str, "%hd", opcode);
      //i8086error(i8086_ERR_STR_ILGOPCODE, str);
      mvwprintw(codeWin, 1+pos, 7, "%s", "ILGOPC"); /* Befehlname ausgeben */
      for (j=pos+2; j<17; j++) /* folgende Zeilen loeschen. */
      {
        wmove(codeWin, j, 2);
        wclrtoeol(codeWin);
      }
      box(codeWin, ACS_VLINE, ACS_HLINE);
      mvwprintw(codeWin, 0, 2, "[Code]");
      wrefresh(codeWin);
      return i8086_ERR_ILGOPCODE;
    }

	cmddata=malloc(10);
	output=malloc(40);
    	cmddata[0]=opcode;
	cmdsize=cmds[opcode]->size;

	for (i=1; i<cmdsize; i++)            /* Parameter byteweise einlesen */
	{
      		cmddata[i] = core->mem[startAdr+read+i];
    	}

	if (cmds[opcode]->hasMod!=0)
	{
		cmdsize+=getAdditionalCmdLength(core, opcode, cmddata[1]);

		for (i=cmds[opcode]->size; i<cmdsize; i++)            /* Parameter byteweise einlesen */
		{
	     		cmddata[i] = core->mem[startAdr+read+i];
	    	}
	}

 	disasm (cmddata, output, 16, startAdr+read, 0, 0);;
	mvwprintw(codeWin, 1+pos, 7, "%s",  output);

	free(output);
	free(cmddata);

	read+=cmdsize;

	if (pos == 14)
		cv->endAdr = startAdr+read;       /* Endadresse fuer naechsten Durchlauf setzen */

    //mvwprintw(codeWin, 1+pos, 7, "%s", cmds[opcode]->name); /* Befehlname ausgeben */
  }


  cv->startAdr = startAdr;              /* Startadresse fuer naechsten Durchlauf setzen */
  box(codeWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(codeWin, 0, 2, "[Code]");
  wrefresh(codeWin);

  return i8086_SUC_EXECCMD;
}


/* Signalhandler fuer Portausgabe	*/
/* Display- und LED-Ports			*/
void portSignalHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  if (hParam==0)
    printLeds(0);
  if (hParam>143 && hParam<159)
    printDisplay(0);
  
  if (oldPortHandler)  
    oldPortHandler(mtyp, hParam, lParam);
}

/* Zeichnet den Kommandorahmen neu */
void clearCmd()
{
  wclear(cmdWin);
  box(cmdWin, ACS_VLINE, ACS_HLINE);
  //mvwprintw(cmdWin, 1, 1, ":");
  mvwprintw(cmdWin, 0, 2, "[Command]");
  wrefresh(cmdWin);
  noecho();
  curs_set(0);
}

/* Setzt den angezeigten Speicheraschnitt  */
unsigned short readMem()
{
  unsigned short v=0;
  mvwprintw(cmdWin, 1, 1, "Adr(Hex max 4 Stellen):");
  wrefresh(cmdWin);

  echo();     /* Echo an  */
  curs_set(1); /* Cursor an*/
  wscanw(cmdWin, "%4hx", &v);

  clearCmd();
  return (v/16)*16;
}

/* Setzt den Wert in einer Speicheradresse */
void setMem()
{
  unsigned short adr=0, v=0;
  mvwprintw(cmdWin, 1, 1, "Adr(Hex max 4 Stellen) Vl(Hex max 2 Stellen)");
  mvwprintw(cmdWin, 2, 1, ":");
  wrefresh(cmdWin);

  echo();     /* Echo an  */
  curs_set(1); /* Cursor an*/
  wscanw(cmdWin, "%4hx %2hx", &adr, &v);

  core->mem[adr] = v;

  clearCmd();
}

/* Setzt die Adresse des Breakpoints	*/
/* Nur ein BP möglich				 	*/
void setBreakpoint()
{
  unsigned short adr=0;
  mvwprintw(cmdWin, 1, 1, "Breakpoint (Hex max 4 Stellen)");
  mvwprintw(cmdWin, 2, 1, ":");
  wrefresh(cmdWin);
  echo();     /* Echo an  */
  curs_set(1); /* Cursor an*/
  wscanw(cmdWin, "%4hx", &adr);
  breakpoint = adr;
  clearCmd();
}

/* Setzt den Wert eines Registers */
void setReg()
{
  unsigned short reg=0, v=0;
  //wmove(cmdWin, 1, 1);
  echo();     /* Echo an  */
  curs_set(1); /* Cursor an*/
  mvwprintw(cmdWin, 1, 1, "Reg(AX=0 BX=3 CX=1 DX=2 SP-DI=4-7 DS 8) Value(Dec):");
  mvwprintw(cmdWin, 2, 1, ":");
  wrefresh(cmdWin);

  wscanw(cmdWin, "%1hi %hi", &reg, &v);
  if (reg==8)
  	i8086SetSegRegister(core, i8086_REG_DS, v);
  else
  	i8086SetRegister(core, reg, 1, v);

  clearCmd();
  printReg();
}

/* Schreibt den Speicher in eine Datei */
int coreDump()
{
  FILE *file;
  int r=0;
  char str[256];

  echo();     /* Echo an  */
  curs_set(1); /* Cursor an*/
  mvwprintw(cmdWin, 1, 1, "Dateiname");
  mvwprintw(cmdWin, 2, 1, ":");
  wrefresh(cmdWin);

  wscanw(cmdWin, "%s", &str);
  
  file = fopen(str, "wb");
  if (file==NULL)
    return i8086_ERR_FILENF;
  
  r = fwrite(core->mem, sizeof(char), i8086_MEM_SIZE, file);

  if (r==0)
    return i8086_ERR_FILEREAD;
        
  fclose(file);
  
  clearCmd();
  return i8086_SUC_FILELOAD; 
}

/* Rahmen zeichnen */
void createWin()
{
  regWin = newwin(5, 43, 0, 0);
  box(regWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(regWin, 0, 2, "[Register]");
  wrefresh(regWin);

  flagWin = newwin(5, 12, 0, 44);
  box(flagWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(flagWin, 0, 2, "[Flags]");
  wrefresh(flagWin);

  cmdWin = newwin(4, 56, 5, 0);
  box(cmdWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(cmdWin, 0, 2, "[Command]");
  wrefresh(cmdWin);

  memWin = newwin(15, 56, 9, 0);
  box(memWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(memWin, 0, 2, "[Memory]");
  wrefresh(memWin);

  ledWin = newwin(3, 22, 21, 57);
  box(ledWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(ledWin, 0, 2, "[LEDS]");
  wrefresh(ledWin);

  switchWin = newwin(3, 22, 18, 57);
  box(switchWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(switchWin, 0, 2, "[SWITCHES]");
  wrefresh(switchWin);

  codeWin = newwin(18, 22, 0, 57);
  box(codeWin, ACS_VLINE, ACS_HLINE);
  mvwprintw(codeWin, 0, 2, "[Code]");
  wrefresh(codeWin);

  if (!(regWin && flagWin && cmdWin && memWin && ledWin && switchWin))
    i8086error(i8086_ERR_STR_CREATE_WIN, i8086_STR_SIZE_TERM);
}

/* Gibt alle verwendeten Ressourcen frei */
void resetAll(int signo)
{  
  if (SIGSEGV==signo)
    i8086error(i8086_ERR_STR_SIGSEGV, "");
    
  endwin();
  i8086close();

  exit(signo);
}


void handleKeyboard(i8086core *core, char c)
{
	switch (c)
	{
		case 'a':
		core->ports.x[0x80]=0x16;
		keydelay=keypressclocks;
		break;
		case 'b':
		core->ports.x[0x80]=0x1E;
		keydelay=keypressclocks;
		break;
		case 'c':
		core->ports.x[0x80]=0x26;
		keydelay=keypressclocks;
		break;
		case 'd':
		core->ports.x[0x80]=0x2E;
		keydelay=keypressclocks;
		break;
		case 'e':
		core->ports.x[0x80]=0x36;
		keydelay=keypressclocks;
		break;
		case 'f':
		core->ports.x[0x80]=0x3E;
		keydelay=keypressclocks;
		break;

 		case '8':
		core->ports.x[0x80]=0x06;
		keydelay=keypressclocks;
		break;


  		case '9':
  		core->ports.x[0x80]=0x0E;
		keydelay=keypressclocks;
		break;

		case '-':
  		//0x03
		core->ports.x[0x80]=0x03;
		keydelay=keypressclocks;
		break;

		case '+':
  		core->ports.x[0x80]=0x0B;
		keydelay=keypressclocks;
		break;

		case 'o':
  		core->ports.x[0x80]=0x13;
		keydelay=keypressclocks;
		break;

		case 'i':
  		core->ports.x[0x80]=0x1B;
		keydelay=keypressclocks;
		break;

		case 't':
  		core->ports.x[0x80]=0x23;
		keydelay=keypressclocks;
		break;

		case 's':
  		core->ports.x[0x80]=0x2B;
		keydelay=keypressclocks;
		break;

		case 'g':
  		core->ports.x[0x80]=0x33;
		keydelay=keypressclocks;
		break;

		case '\n':
  		core->ports.x[0x80]=0x3B;
		keydelay=keypressclocks;
		break;

		default:
		if (c>='0' && c<='7')
		{
			core->ports.x[0x80]=(atoi(&c)<<3)|5 ;
			keydelay=keypressclocks;
		}
	}

   if (keydelay>0)
   	keydelay--;
   else
   	{//core->ports.x[0x80]++;
		//if (prellbock%8==0)

		{
			prellbock+=8;
			prellbock=prellbock | 7;
			core->ports.x[0x80]=prellbock;// & 56;
		}
		//prellbock++;
	}
}

/* Laed das ROM-File in den Speicher ab Adresse adr */
void LoadRomFile(const char *filename, unsigned short adr)
{
  if (i8086loadBinFile(core, filename, adr)!=i8086_SUC_FILELOAD)
    i8086error(filename, i8086_ERR_STR_FILENF);
    args |= LOAD_ROM;
}

/* Laed den Core-Dump */
void LoadCoreDumpFile(const char *filename)
{
  if (i8086loadBinFile(core, filename, 0)!=i8086_SUC_FILELOAD)
    i8086error(filename, i8086_ERR_STR_FILENF);
}

int main(int argc, char *argv[])
{
  //int ret;
  int c;
  unsigned short i=0, ac;
  unsigned short rom_adr, startAdr=i8086_BEGINADR;  /* Mem-Viewer Startadresse  */
  char str[6];
  char cfgStr[i8086_CFG_MAX_VALUE_LEN];
  unsigned short adr;
  codeView cv;                             /* Code-Viewer Data */

  if (argc<2)
  {
                    printf("i8086emu "VERSION_NUMBER);
                    printf("\nUsage: %s [OPTIONS] FILENAME\n", argv[0]);
                    printf("\nOptions:");
                    printf("\n -c\t\t\tCodeViewer deaktivieren");
                    printf("\n -o XXXXh\t\tStartadresse");
                    printf("\n -r XXXXh\t\tStartadresse ROM-File");
                    printf("\n -d file\t\tDUMP-File");
                    printf("\n --version\t\tshow version information");
                    printf("\n --help\t\t\tthis help information");
                    printf("\n");

    exit(1);
  }
  
  
  signal(SIGTERM, resetAll);
  signal(SIGINT, resetAll);
  signal(SIGQUIT, resetAll);
  signal(SIGSEGV, resetAll);
  
  i8086clearLog(); /* Log-File leeren. */
  i8086init(); /* Prozessor initialisieren. */
  oldPortHandler = i8086SetMsgFunc(i8086_SIG_PORT_OUT, portSignalHandler);

  /* Config-File laden */
  if (i8086ReadStrConfig(cfgStr, CONFIG_FILE, "ROMFILE")!=0)
  {
    rom_adr = i8086ReadHexConfig(CONFIG_FILE, "ROMSTARTADR", 0xc000);
    LoadRomFile(cfgStr, rom_adr);
  }
  else /* Wenn ROM -> kein Core-Dump */
    if (i8086ReadStrConfig(cfgStr, CONFIG_FILE, "COREDUMP")!=0)
      LoadCoreDumpFile(cfgStr);
  startAdr = i8086ReadHexConfig(CONFIG_FILE, "PROGSTARTADR", 0x0100);
    
  for (ac=1; ac<argc; ac++) /* Programmargumente auswerten und speichern */
  {
    char *str;
    
    if(strcmp(argv[ac],"--version")==0)
    {
                    printf("i8086emu "VERSION_NUMBER"\n"
                    "Copyright (C) 2004 JMH, RD, FB, CST\n"
                    "i8086emu comes with NO WARRANTY,\n"
                    "to the extent permitted by law.\n"
                    "You may redistribute copies of i8086emu\n"
                    "under the terms of the GNU General Public License.\n");
                    exit(1);
    }
    if(strcmp(argv[ac],"--help")==0)
    {
                    printf("i8086emu "VERSION_NUMBER);
                    printf("\nUsage: %s [OPTIONS] FILENAME\n", argv[0]);
                    printf("\nOptions:");
                    printf("\n -c\t\t\tCodeViewer deaktivieren");
                    printf("\n -o XXXXh\t\tStartadresse");
                    printf("\n -r XXXXh\t\tStartadresse ROM-File");
                    printf("\n -d file\t\tDUMP-File");
                    printf("\n --version\t\tshow version information");
                    printf("\n --help\t\t\tthis help information");
                    printf("\n");

    exit(1);
    }          
    if (strcmp(argv[ac], "-c")==0)          /* CodeViewer deaktivieren */
      args |= HIDE_CODE_VIEWER;
    if ((str=strstr(argv[ac], "-o"))!=NULL) /* Startadresse des Programmes setzen */
    {
      str = strtok(str, "-o");
      if (str!=NULL)
        startAdr = strtoul(str, NULL, 16);
    }
    if ((str=strstr(argv[ac], "-r"))!=NULL) /* ROM laden */
    {
      //int adr;
      if (argc<ac+2)
      {
        printf("\n -r XXXXh file -> Startadresse ROM-File");
        exit(1);
      }
      rom_adr = strtoul(argv[ac+1], NULL, 16);
      LoadRomFile(argv[ac+2], rom_adr);
    }
    if ((str=strstr(argv[ac], "-d"))!=NULL) /* Core-Dump laden */
    {
      if (argc<ac+1)
      {
        printf("\n -d file -> DUMP-File");
        exit(1);
      }       
      LoadCoreDumpFile(argv[ac+1]);
    }
  }

  if (i8086loadBinFile(core, argv[argc-1], startAdr)!=i8086_SUC_FILELOAD)
  {
    i8086error(argv[argc-1], i8086_ERR_STR_FILENF);    
  }
  if (args & LOAD_ROM) /* ROM verarbeiten */
    startAdr = rom_adr;
  core->pc = startAdr;
    
  initscr();
  clear();
  refresh();
  createWin();

  printReg();
  printFlags();
  printLeds(0);
  printSwitches();
  printDisplay();
  printMem(startAdr);
  cv.startAdr = 0;//i8086_BEGINADR;
  cv.endAdr = 0;

  printCode(core, commands, &cv);
  //i8086PushMsg(1, 1);

  noecho();     /* getch echo aus */
  curs_set(0);  /* Cursor ausschalten */

  keypad(stdscr,1);
  //while (/*c=='g' ||*/ (c=getch/*ar*/())!='q')

  //sendepuffer ist leer
  //core->ports.x[0xD2]=3;

  while ((c==ERR && i%50000!=0) || (c=getch/*ar*/())!='q') //automode nur aller 50000 takte abbrechbar, reicht aber aus, sonst wars zu langsam
  {
    i++;
    if (c==KEY_F(9))
      nodelay(stdscr,TRUE);
    else if (c=='n')
      nodelay(stdscr,FALSE);
    else if (c==KEY_F(8))
    {
    	adr=i8086GetSegRegister_fast(core,i8086_REG_CS, 1)+core->pc;
    	stepover=adr+commands[core->mem[adr]]->size;
	if (commands[core->mem[adr]]->hasMod!=0)
		stepover+=getAdditionalCmdLength(core, core->mem[adr], core->mem[adr+1]);
    	nodelay(stdscr,TRUE);
    }

    if (c==ERR||c=='n') /* next Opcode */
      if (i8086execCommand(core, commands)==i8086_ERR_ILGOPCODE)
      {
        sprintf(str, "%hd", core->mem[core->pc + i8086GetSegRegister_fast(core, i8086_REG_CS, 1)]);
        i8086error(i8086_ERR_STR_ILGOPCODE, str);
        //resetAll(0);
      }

    if (i8086GetSegRegister_fast(core,i8086_REG_CS, 1)+core->pc==breakpoint || i8086GetSegRegister_fast(core,i8086_REG_CS, 1)+core->pc==stepover)
    {
    	i=0;
    	nodelay(stdscr,FALSE);
	    c=0;
    }

    if (c=='r') /* set Registervalue */
      setReg();
    else if (c==KEY_F(1)) /* Print Help */
      printHelp();
    else if (c==KEY_F(2)) //Breakpoint setzen
      {
      	setBreakpoint();
	c=0;
      }
    else if (c=='m') /* Anfangsadresse von Speicherviewer setzen */
      startAdr=readMem();
    else if (c=='w') /* Wert von Speicherzelle setzen */
      setMem();
    else if (c==KEY_F(3)) /* CoreDump */
      coreDump();
    else if ((c>='0')&&(c<='7')) /* Schalter An/Aus */
    {
      unsigned char dual[] = {1,2,4,8,16,32,64,128};
      core->ports.x[0] = core->ports.x[0] ^ dual[atoi((char*)&c)];
    }
    else if (c==KEY_F(12))	//Reset Taste
    {
    	i=0;
    	core->pc=0x0c000;
	i8086SetSegRegister(core,i8086_REG_CS,0xc00);
	i8086SetRegister(core,i8086_REG_AX,1,0);
	//i8086SetRegister(core,i8086_REG_BX,1,0);
	//i8086SetRegister(core,i8086_REG_CX,1,0);
	//i8086SetRegister(core,i8086_REG_DX,1,0);
	//nodelay(stdscr,TRUE);
    }

    handleKeyboard(core,c);

    if (c!=ERR)
    {
      printReg();
      printFlags();
      printSwitches();
      printDisplay();
      printMem(startAdr);
      printCode(core, commands, &cv);
    //printLeds();
    }
  }

  resetAll(0);

  return EXIT_SUCCESS;
}

