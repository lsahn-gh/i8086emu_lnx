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
/* Hilfsfunktionen für die Emulatorfunktionen       */
/*                                                  */
/****************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "i8086datatrans.h"
#include "i8086arithmetic.h"
#include "i8086proc.h"
#include "i8086control.h"
#include "i8086logic.h"
#include "i8086strings.h"
#include "i8086controltrans.h"
#include "i8086error.h"
#include "i8086wrapper.h"
#include "i8086util.h"

/* Autor: JMH */
/* Erstellt eine neue i8086command*-Struktur               */
/* name: Name des ASM-Befehls + \0.                        */
/*      (Max. Laenge des Namen + \0) = i8086_OPC_NAME_LEN  */
i8086command* i8086AllocCommnand(char name[i8086_OPC_NAME_LEN], unsigned char opcode,
                                 unsigned char size, unsigned char hasMod, unsigned char clocks,
                                 void (*func)(i8086core *core, unsigned char command, i8086Parameter para, i8086Parameter data)
                                )
{
  i8086command* cmd;

  cmd = (i8086command*)malloc(sizeof(i8086command));
  strcpy(cmd->name, name);
  cmd->opcode = opcode;
  cmd->size = size;
  cmd->hasMod = hasMod;
  cmd->clocks = clocks;
  cmd->func = func;
  return cmd;
}

/* Autor: JMH */
/* Erstellt eine neue i8086command*-Struktur,              */
/* fuegt sie in das Kommando-Array cmds ein und            */
/* gibt sie zurück.                                        */
/* name: Name des ASM-Befehls + \0.                        */
/*      (Max. Laenge des Namen + \0) = i8086_OPC_NAME_LEN  */
i8086command* i8086AddCommand(char name[i8086_OPC_NAME_LEN], unsigned char opcode,
                              unsigned char size, unsigned char hasMod, unsigned char clocks,
                              void (*func)(i8086core *core, unsigned char command, i8086Parameter para, i8086Parameter data),
                              i8086command **cmds
                             )
{
  if (cmds[opcode]==NULL)
  {
    cmds[opcode] = i8086AllocCommnand(name, opcode, size, hasMod, clocks, func);
    //return cmds[opcode];
  }
  else
  {
    char str[6];
    sprintf(str, "%hd", opcode);
    i8086error(i8086_STR_OPCODE_ALR_EXIST, str);
    //return 0;
  }
  return cmds[opcode];
}


/* Autor: RD */
void memWriteByte(i8086core *core, i808616BitAdr adr, char val, int reg) //schreibt ein Byte an Adresse adr in den Speicher
{
	doSegmentOverride(core,reg);
	core->mem[adr+i8086GetSegRegister_fast(core,reg,1)]=val;
}

/* Autor: RD */
void memWriteWord(i8086core *core, i808616BitAdr adr, unsigned short val, int reg) //schreibt ein Word an Adresse adr in den Speicher
{
	doSegmentOverride(core,reg);
	core->mem[i8086GetSegRegister_fast(core,reg,1)+adr]=val;
	core->mem[i8086GetSegRegister_fast(core,reg,1)+adr+1]=swapbytes(val);
}

/* Autor: RD */
void memWrite(i8086core *core, i808616BitAdr adr, unsigned short val, unsigned char w, int reg) //schreibt 16/8bit (abhängig von w) aus dem Speicher
{
	doSegmentOverride(core,reg);
	core->mem[i8086GetSegRegister_fast(core,reg,1)+adr]=val;
	if (w)
		core->mem[i8086GetSegRegister_fast(core,reg,1)+adr+1]=swapbytes(val);
}

/* Autor: RD */
unsigned short memRead(i8086core *core, i808616BitAdr adr, unsigned char w, int reg) //liest 16/8bit (abhängig von w) aus dem Speicher
{
	unsigned short wd=0;
	doSegmentOverride(core,reg);
	wd=wd|core->mem[i8086GetSegRegister_fast(core,reg,1)+adr];
	if (w)
		wd=wd|(core->mem[i8086GetSegRegister_fast(core,reg,1)+adr+1]<<8);
	return wd;
}

/* Autor: RD */
char memReadByte(i8086core *core, i808616BitAdr adr, int reg)		//liest ein Byte an Adresse adr aus dem Speicher
{
	doSegmentOverride(core,reg);
	return core->mem[i8086GetSegRegister_fast(core,reg,1)+adr];
}

/* Autor: RD */
unsigned short memReadWord(i8086core *core, i808616BitAdr adr, int reg)	//liest ein Word an Adresse adr aus dem Speicher
{
	unsigned short w=0;
	doSegmentOverride(core,reg);
	w=w|core->mem[i8086GetSegRegister_fast(core,reg,1)+adr];
	w=w|(core->mem[i8086GetSegRegister_fast(core,reg,1)+adr+1]<<8);
	return w;
}

/* Autor: RD */
/* liefert die zusaetzliche Lenge von Befehlen mit Speicheradressen */
unsigned char getAdditionalCmdLength(i8086core *core, unsigned char opcode, char modrm)
{
	char mod;
	char rm;
	char length=0;
	mod=getBitSnipped(modrm,7,2);
	rm=getBitSnipped(modrm,2,3);

	switch (mod)
	{
		case 0:
			if (rm==6)
				length=2;
			else
				length=0;
			break;
		case 1:
			length=1;
			break;
		case 2:
			length=2;
			break;
		case 3:
			length=0;
	}

	//TEST hat eine andere Länge als Befehle mit dem gleichen Opcode
	if (opcode==0xF7 && getBitSnipped(modrm,5,3)==0)
		length+=2;
	else if (opcode==0xF6 && getBitSnipped(modrm,5,3)==0)
		length++;

	return length;
}

/* Autor: RD */
/* berechnet die Speicheradresse auf r/m und mod */
/* siehe PDFs für genauere Beschreibung */
i808616BitAdr decodeMemAdr(i8086core *core, char modrm, signed short disp)
{
	char mod;
	char rm;
	i8086SingleRegister bx;
	i8086SingleRegister si;
	i8086SingleRegister di;
	i8086SingleRegister bp;

	i808616BitAdr adr;
	signed char disp8;
	unsigned short disp16;

	signed short offset;
	disp16=swapbytes(disp);
	disp8=(signed char) (disp >> 8);;


	mod=getBitSnipped(modrm,7,2);
	rm=getBitSnipped(modrm,2,3);

	i8086GetRegisterRef(&bx, core, 1, i8086_REG_BX);
	i8086GetRegisterRef(&si, core, 1, i8086_REG_SI);
	i8086GetRegisterRef(&di, core, 1, i8086_REG_DI);
	i8086GetRegisterRef(&bp, core, 1, i8086_REG_BP);


	if (mod==0 && rm==6)
	{
		adr=disp16;
	}
	else
	{

	switch (mod)
	{
		case 0:
			offset=0;
			break;
		case 1:
			offset=disp8;
			break;
		case 2:
			offset=disp16;
			break;
	}
	switch (rm)
	{
		case 0:
			adr= bx.x+si.x+offset;
			break;
		case 1:
			adr= bx.x+di.x+offset;
			break;
		case 2:
			adr= bp.x+si.x+offset;
			if (core->miscData.segoverride==-1) core->miscData.segoverride=i8086_REG_SS;
			break;
		case 3:
			adr= bp.x+di.x+offset;
			if (core->miscData.segoverride==-1) core->miscData.segoverride=i8086_REG_SS;
			break;
		case 4:
			adr= si.x+offset;
			break;
		case 5:
			adr= di.x+offset;
			break;
		case 6:
			adr= bp.x+offset;
			if (core->miscData.segoverride==-1) core->miscData.segoverride=i8086_REG_SS;
			break;
		case 7:
			adr= bx.x+offset;
			break;
	}
	}

	return adr;
}

/* Autor: RD */
/* w==0 8bit, w==1 16bit */
//korrigiert Overflow- und Carryfkag noch einer Addtition/Subtraktion
//die summanden und die summe sind zu uebergeben
//a +/- b = c;  w=1 16bit, w=0 8bit, addsub=1 addtition, addsub=0 subtraktion
void correctOvCaFlagafterAddSub(i8086core *core, unsigned short a, unsigned short b, unsigned short c, unsigned char w, unsigned char addsub)
{
	unsigned short bit,tmp;

	if (w)
		bit=32768;
	else
		bit=128;

	if (! addsub) b=-b;

	//Aux Flag
	//if ( (addsub && ((a + b) > (a|0xF))) || (!addsub && ((a - b) < (a&~0xF))) )
	if ( ((addsub && ((a&0xF) + (b&0xF) > 0xF)) || ((!addsub && ((a&0xF) - (b&0xF)) < 0))) )
		core->flags|=i8086_FLAG_ACF;
	else
		core->flags&=~i8086_FLAG_ACF;

	//if (addsub) //bei addition
	{
		if (!(a&bit) && !(b&bit) && !(c&bit)) //+++
			{
				core->flags=core->flags & (~i8086_FLAG_SF);
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags & (~i8086_FLAG_OF);
			}

		else if (!(a&bit) && !(b&bit) && c&bit)	//++-
			{
				core->flags=core->flags | i8086_FLAG_SF;
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags | i8086_FLAG_OF;
			}

		else if (a&bit && !(b&bit) && !(c&bit))	//-++
			{
				core->flags=core->flags & (~i8086_FLAG_SF);
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags & (~i8086_FLAG_OF);
			}

		else if (a&bit && !(b&bit) && c&bit)	//-+-
			{
				core->flags=core->flags | i8086_FLAG_SF;
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags & (~i8086_FLAG_OF);
			}

	/*}
	else //bei negativen summanden
	{*/
		else if (!(a&bit) && b&bit && !(c&bit))	//+-+
			{
				core->flags=core->flags & (~i8086_FLAG_SF);
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags | i8086_FLAG_OF;
			}

		else if (!(a&bit) && b&bit && c&bit)	//+--
			{
				core->flags=core->flags | i8086_FLAG_SF;
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags | i8086_FLAG_OF;
			}

		else if (a&bit && b&bit && !(c&bit))	//--+
			{
				core->flags=core->flags & (~i8086_FLAG_SF);
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags & (~i8086_FLAG_OF);
			}
		else if (a&bit && b&bit && c&bit)		//---
			{
				core->flags=core->flags | i8086_FLAG_SF;
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags | i8086_FLAG_OF;
			}
	}

	//bei Subtraktion werden Carry und Overflow Flag vertauscht
	//das hab ich mit ner Tabelle überprüft
	if (!addsub)
	{
		tmp=core->flags;

		if (core->flags & i8086_FLAG_OF)
			core->flags=core->flags | i8086_FLAG_CF;
		else
			core->flags=core->flags & (~i8086_FLAG_CF);

		if (tmp & i8086_FLAG_CF)
			core->flags=core->flags | i8086_FLAG_OF;
		else
			core->flags=core->flags & (~i8086_FLAG_OF);
	}
}

/* Autor: RD */
/* setzt ZeroFlag entsprechend i */
/* w==0 8bit, w==1 16bit */
void correctZeroFlag(i8086core *core, unsigned short i, unsigned char w) //setzt ZeroFlag entsprechend Wert
{
	unsigned short j=i;
	if (!w)
		j=j&255;
	if (!j)
		core->flags=core->flags | i8086_FLAG_ZF;
	else
		core->flags=core->flags & (~i8086_FLAG_ZF);
}

/* Autor: RD */
/* setzt SignFlag entsprechend i */
/* w==0 8bit, w==1 16bit */
void correctSignFlag(i8086core *core, unsigned short i,unsigned char w)
{
	char msb; //most sig. bit
	if (w)
		msb=(i&32768)>>15;
	else
		msb=(i&128);

	if (msb)
	core->flags=core->flags | i8086_FLAG_SF;
	else
	core->flags=core->flags & (~i8086_FLAG_SF);
}

/* Autor: RD */
/* setzt PF in Abhängigkeit der unteren 8 Bit von i*/
void correctParityFlag(i8086core *core, unsigned short i)
{
	char lsb=i; //least sig. byte
	char bitcount=0;
	if (lsb&1) bitcount++;
	if (lsb&2) bitcount++;
	if (lsb&4) bitcount++;
	if (lsb&8) bitcount++;
	if (lsb&16) bitcount++;
	if (lsb&32) bitcount++;
	if (lsb&64) bitcount++;
	if (lsb&128) bitcount++;

	if (bitcount % 2)
	core->flags=core->flags & (~i8086_FLAG_PF);
	else
	core->flags=core->flags | i8086_FLAG_PF;
}

/* Autor: RD */
/* setzt Zero-, Parity- und SignFlag enstprechend i */
/* w==0 8bit, w==1 16bit */
void correctArithmeticFlags(i8086core *core, unsigned short i,unsigned char w)
{
	correctZeroFlag(core,i,w);
	correctParityFlag(core,i);
	correctSignFlag(core,i,w);
}


/*CST*/
/*falls type=i8086_INPUT_PORT bezieht sich die folgende operation auf die INPUT-Ports*/
/*falls type=i8086_OUTPUT_PORT bezieht sich die folgende operation auf die OUTPUT-Ports*/
/*falls rw=i8086_READ_PORT wird das Byte zurück gegeben welches an Port portNum anliegt*/
/*falls rw=i8086_WRITE_PORT wird das Byte "data" an den Port portNum angelegt*/
/*Im Fehlerfall wird i8086_ERR_ILGPORT zurückgegeben*/
unsigned char portOpByte(i8086core *core,unsigned char type,unsigned char data,unsigned short portNum, unsigned char rw)
{
        if(portNum<i8086_PORTS)//ist der port vorhanden
        {
                if(type) // Operation auf OUTPUT-Ports
                        portNum+=i8086_PORTS;//aktuelle portnummer ist portNum + i8086_PORTS - da es sich um OUTPUT-port handelt
                if(rw) //es soll data an den port gegeben werden
                {
                        core->ports.x[portNum]=data;//schreibe data an den Port - INPUT Ports von 0...i8086_PORTS
                        return 0;//gib nichts zurück
                }
                else //es soll 1 byte vom port gelesen werden
                {
                        return core->ports.x[portNum];//gibt das Byte am Port zurück - INPUT Ports von 0...i8086_PORTS
                }
        }
	else//illegaler port
                return i8086_ERR_ILGPORT; //error
}

/*CST*/
/*falls type=i8086_INPUT_PORT bezieht sich die folgende operation auf die INPUT-Ports*/
/*falls type=i8086_OUTPUT_PORT bezieht sich die folgende operation auf die OUTPUT-Ports*/
/*falls rw=i8086_READ_PORT wird das word zurück gegeben welches an Port portNum anliegt*/
/*falls rw=i8086_WRITE_PORT wird das word "data" an den Port portNum angelegt*/
/*Im Fehlerfall wird i8086_ERR_ILGPORT zurückgegeben*/
unsigned short portOpWord(i8086core *core,unsigned char type,unsigned short data, unsigned short portNum, unsigned char rw)
{
 if(portNum<i8086_PORTS)//ist der port vorhanden
        {
                if(type) // Operation auf OUTPUT-Ports
                        portNum+=i8086_PORTS; //aktuelle portnummer ist portNum + i8086_PORTS - da es sich um OUTPUT-port handelt


                if(rw) //es soll data an den port gegeben werden
                {
                                core->ports.x[portNum]=data;//schreibe data an den Port - INPUT Ports von 0...i8086_PORTS
                        return 0;//gib nichts zurück
                }
                else //es soll 1 word vom port gelesen werden
                {
                        return core->ports.x[portNum];//gibt das word am Port zurück - INPUT Ports von 0...i8086_PORTS
                }
        }
	else//illegaler port
                return i8086_ERR_ILGPORT;//error
}

/*Autor: RD*/
/* legt einen Wert auf den Stack */
void push(i8086core *core, unsigned short x)
{
	i8086SingleRegister stackreg;
	i8086GetRegisterRef(&stackreg, core, 1, i8086_REG_SP); //stackregister

	stackreg.x-=2; /* dekrementiere stackpointer um 2 */
	memWriteWord(core,stackreg.x,x,i8086_REG_SS);

	i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x);
}

/*Autor: RD*/
/* holt einen Wert vom Stack */
unsigned short pop(i8086core *core)
{
	unsigned short result;
	i8086SingleRegister stackreg;
	i8086GetRegisterRef(&stackreg, core, 1, i8086_REG_SP); //stackregister

	result=memReadWord(core,stackreg.x,i8086_REG_SS);
	stackreg.x+=2;		//inkrementiere stackpointer um 2

	i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x);

	return result;
}

/***************************************************************/
/***************************************************************/

void i8086InitCommands(i8086command **cmds)
{
  unsigned char i;
  /* Datentransfer */
  /*              Name      OpC Sz Mod Clk   EmuFkt     cmds                  */
  for(i=80;i<=87;i++)
	  i8086AddCommand("push", i, 1, 0, 11, i8086PushReg, cmds); // push reg16
  for(i=88;i<=95;i++)
	  i8086AddCommand("pop",  i, 1, 0, 8, i8086PopReg, cmds); // pop reg16
  for(i=6;i<=30;i+=8)
	  i8086AddCommand("push", i, 1, 0, 10, i8086PushReg, cmds); // push segmentreg
  for(i=7;i<=31;i+=8)
	  i8086AddCommand("pop",  i, 1, 0, 8, i8086PopReg, cmds); // pop segmentreg
	  
  i8086AddCommand("xchg", 134, 2,1, 10, i8086XchgReg, cmds); // xchg regb,regb / regb,mem
  i8086AddCommand("xchg", 135, 2, 1, 10, i8086XchgReg, cmds); // xchg regw,regw / regw,mem
  i8086AddCommand("mov" , 136, 2, 1, 2, i8086MovReg,  cmds); // mov regb,regb mov mem,reg8
  i8086AddCommand("mov" , 137, 2, 1, 2, i8086MovReg,  cmds); // mov regw,regw mov mem,reg16
  i8086AddCommand("mov" , 138, 2, 1, 9, i8086MovReg,  cmds); // mov reg8,mem
  i8086AddCommand("mov" , 139, 2, 1, 8, i8086MovReg,  cmds); // mov reg16,mem
  i8086AddCommand("mov" , 140, 2, 1, 8, i8086MovSeg,  cmds); // mov mem16/reg16,segreg
  i8086AddCommand("lea" , 141, 2, 1, 2, i8086Lea,  cmds); // lea reg, mem
  i8086AddCommand("mov" , 142, 2, 1, 8, i8086MovSeg,  cmds); // mov segreg,mem16/reg16
  i8086AddCommand("mov" , 160, 3, 0, 10, i8086MovAcc,  cmds); // mov acc8,mem
  i8086AddCommand("mov" , 161, 3, 0, 10, i8086MovAcc,  cmds); // mov acc16,mem
  i8086AddCommand("mov" , 162, 3, 0, 10, i8086MovAcc,  cmds); // mov mem,acc8
  i8086AddCommand("mov" , 163, 3, 0, 10, i8086MovAcc,  cmds); // mov mem,acc16

  for(i=176;i<=183;i++)
  	i8086AddCommand("mov" , i, 2, 0, 10, i8086MovConst,  cmds); // mov reg8,const
  for(i=184;i<=191;i++)
  	i8086AddCommand("mov" , i, 3, 0, 10, i8086MovConst,  cmds); // mov reg16,const
  i8086AddCommand("les" , 196, 2, 1, 16, i8086LdsLes,  cmds); //les reg,mem
  i8086AddCommand("lds" , 197, 2, 1, 16, i8086LdsLes,  cmds); //lds reg,mem
  i8086AddCommand("mov" , 198, 3, 1, 10, i8086MovConst,  cmds); // mov mem8,const
  i8086AddCommand("mov" , 199, 4, 1, 10, i8086MovConst,  cmds); // mov mem16,const



  for(i=145;i<=151;i++)
	i8086AddCommand("xchg", i, 1,0, 3, i8086XchgReg, cmds); // xchg reg,acc / acc,reg
  i8086AddCommand("pushf", 156, 1, 0, 10, i8086Pushf,    cmds); // pushf
  i8086AddCommand("popf", 157, 1, 0, 8, i8086Popf,    cmds); // popf
  i8086AddCommand("sahf", 158, 1, 0, 4, i8086Sahf,    cmds); // sahf
  i8086AddCommand("lahf", 159, 1, 0, 4, i8086Lahf,    cmds); // lahf
  i8086AddCommand("xlat", 215, 1, 0, 11, i8086Xlat,    cmds); // xlat
/*  i8086AddCommand("push", 255, 2, 1, 16, i8086PushMem,    cmds); // push mem16 *** ersetzt durch i8086WrapJmpPushInc *** */
  i8086AddCommand("PshIcJ", 255, 2, 1, 16, i8086WrapJmpPushInc, cmds); // push mem16
  i8086AddCommand("pop", 143, 2, 1, 17, i8086PopMem,    cmds); // pop mem16
  i8086AddCommand("in", 228, 2, 0, 10, i8086In,    cmds); // in al,port
  i8086AddCommand("in", 229, 2, 0, 14, i8086In,    cmds); // in ax,port
  i8086AddCommand("out", 230, 2, 0, 10, i8086Out,    cmds); // out port,al
  i8086AddCommand("out", 231, 2, 0, 14, i8086Out,    cmds); // out port,ax
  i8086AddCommand("in", 236, 1, 0, 8, i8086In,    cmds); // in al, port aus DX
  i8086AddCommand("in", 237, 1, 0, 12, i8086In,    cmds); // in ax, port aus DX
  i8086AddCommand("out", 238, 1, 0, 8, i8086Out,    cmds); // out port aus DX,al
  i8086AddCommand("out", 239, 1, 0, 12, i8086Out,    cmds); // out port aus DX,ax



  //arithmetik
  /*                  Name   OpC Sz Mod Clk   EmuFkt       cmds                  */
  for (i=0;i<=3;i++)
    i8086AddCommand("add", i, 2, 1,  3, i8086AddRegMem, cmds); //add Reg./Memory with Register to Either
  for (i=16;i<=19;i++)
    i8086AddCommand("adc", i, 2, 1,  3, i8086AddRegMem, cmds); //adc Reg./Memory with Register to Either
  for (i=40;i<=43;i++)
    i8086AddCommand("sub", i, 2, 1,  3, i8086AddRegMem, cmds); //sub Reg./Memory with Register to Either
  for (i=56;i<=59;i++)
    i8086AddCommand("cmp", i, 2, 1,  3, i8086AddRegMem, cmds); //adc Reg./Memory with Register to Either
  for (i=24;i<=27;i++)
    i8086AddCommand("sbb", i, 2, 1,  3, i8086AddRegMem, cmds); //sbb Reg./Memory with Register to Either

  i8086AddCommand("add", 5, 3, 0,  4, i8086AddAcc, cmds); //add accu,immediate8
  i8086AddCommand("add", 4, 2, 0,  4, i8086AddAcc, cmds); //add accu,immediate16

  i8086AddCommand("adc", 21, 3, 0,  4, i8086AddAcc, cmds); //adc accu,immediate8
  i8086AddCommand("adc", 20, 2, 0,  4, i8086AddAcc, cmds); //adc accu,immediate16

  i8086AddCommand("sub", 45, 3, 0,  4, i8086AddAcc, cmds); //sub accu,immediate8
  i8086AddCommand("sub", 44, 2, 0,  4, i8086AddAcc, cmds); //sub accu,immediate16

  i8086AddCommand("sbb", 29, 3, 0,  4, i8086AddAcc, cmds); //sbb accu,immediate8
  i8086AddCommand("sbb", 28, 2, 0,  4, i8086AddAcc, cmds); //sbb accu,immediate16

  i8086AddCommand("cmp", 61, 3, 0,  4, i8086AddAcc, cmds); //cmp accu,immediate8
  i8086AddCommand("cmp", 60, 2, 0,  4, i8086AddAcc, cmds); //cmp accu,immediate16

  /*	musste gewrapped werden durch darunter stehenden Code
  i8086AddCommand("add", 128, 3, 1,  4, i8086AddImmed, cmds); //add reg8/mem8, immediate 8
  i8086AddCommand("add", 129, 4, 1,  4, i8086AddImmed, cmds); //add reg/mem, immediate16
  i8086AddCommand("add", 131, 3, 1,  4, i8086AddImmed, cmds); //add reg/mem,,immediate8
  i8086AddCommand("add", 130, 3, 1,  4, i8086AddImmed, cmds); //add reg/mem,,immediate8
	*/
  i8086AddCommand("AdSbOC", 128, 3, 1, 4, i8086WrapAddAdcSubSsbCmpAndOrXor, cmds); // reg8/mem8, immediate 8
	i8086AddCommand("AdSbOC", 129, 4, 1, 4, i8086WrapAddAdcSubSsbCmpAndOrXor, cmds); // reg/mem, immediate16
  i8086AddCommand("AdSbOC", 130, 3, 1, 4, i8086WrapAddAdcSubSsbCmpAndOrXor, cmds); // reg/mem,immediate8
	i8086AddCommand("AdSbOC", 131, 3, 1, 4, i8086WrapAddAdcSubSsbCmpAndOrXor, cmds); // reg/mem,immediate8


  for (i=64;i<=71;i++)
    {i8086AddCommand("inc", i, 1, 0,  3, i8086IncReg,    cmds);} //inc regx
  for (i=72;i<=79;i++)
    {i8086AddCommand("dec", i, 1, 0,  3, i8086DecReg,    cmds);} //dec regx

  i8086AddCommand("inc/d", 254, 2, 1,  3, i8086IncDecRegMem,  cmds); //inc/deg Reg./Memory
/*  i8086AddCommand("inc/d", 255, 2, 1,  3, i8086IncDecRegMem,  cmds); //inc/deg Reg./Memory *** ersetzt durch i8086WrapJmpPushInc ***  */

  i8086AddCommand("cbw", 152, 1, 0,  2, i8086ConvertByteWord,  cmds); //cbw
  i8086AddCommand("cwd", 153, 1, 0,  5, i8086ConvertByteDWord,  cmds); //cwd
  i8086AddCommand("aaa", 55, 1, 0,  8, i8086AAA,  cmds); //aaa
  i8086AddCommand("aas", 63, 1, 0,  8, i8086AAS,  cmds); //aas
  i8086AddCommand("aad", 213, 2, 0,  60, i8086AAD,  cmds); //aad
  i8086AddCommand("aam", 212, 2, 0,  83, i8086AAM,  cmds); //aam
  i8086AddCommand("daa", 39, 1, 0,  4, i8086DAA,  cmds); //daa
  i8086AddCommand("das", 47, 1, 0,  4, i8086DAS,  cmds); //das

  //logic
  /*                  Name   OpC Sz Mod Clk   EmuFkt       cmds                  */

  i8086AddCommand("NgNtMl", 246, 2, 1,  2, i8086WrapNegNotMulDivTest, 	 cmds); // reg8-reg8
  i8086AddCommand("NgNtMl", 247, 2, 1,  2, i8086WrapNegNotMulDivTest, 	 cmds); // reg16-reg16

  for (i=208;i<=211;i++) /* ShlSalShrRolRorRclRcr */
  	{i8086AddCommand("ShRo", i, 2, 1,  7, i8086ShlSalShrRolRorRclRcr,cmds);}
  for (i=48;i<=51;i++)
  	{i8086AddCommand("xor", i, 2, 1,  10, i8086AndTestOrXorRegMem,cmds);} // Xor Reg-Mem
  	i8086AddCommand("xor", 52, 2, 0,  9, i8086AndTestOrXorAccumulator,cmds); // Xor Reg/Mem-1Immed
  	i8086AddCommand("xor", 53, 3, 0,  9, i8086AndTestOrXorAccumulator,cmds); // Xor Reg/Mem-2Immed
  for (i=32;i<=35;i++)
  	{i8086AddCommand("and", i, 2, 1,  10, i8086AndTestOrXorRegMem,cmds);} // and Reg-Mem
  	i8086AddCommand("and", 36, 2, 0,  9, i8086AndTestOrXorAccumulator,cmds); // and Reg/Mem-1Immed
  	i8086AddCommand("and", 37, 3, 0,  9, i8086AndTestOrXorAccumulator,cmds); // and Reg/Mem-2Immed
  for (i=8;i<=11;i++)
  	{i8086AddCommand("or", i, 2, 1,  10, i8086AndTestOrXorRegMem,cmds);} // or Reg-Mem
  	i8086AddCommand("or", 12, 2, 0,  9, i8086AndTestOrXorAccumulator,cmds); // or Reg/Mem-1Immed
  	i8086AddCommand("or", 13, 3, 0,  9, i8086AndTestOrXorAccumulator,cmds); // or Reg/Mem-2Immed
  for (i=132;i<=133;i++)
  	{i8086AddCommand("test", i, 2, 1,  10, i8086AndTestOrXorRegMem,cmds);} // test Reg-Mem
  	i8086AddCommand("test", 168, 2, 0,  9, i8086AndTestOrXorAccumulator,cmds); // test Reg/Mem-1Immed
  	i8086AddCommand("test", 169, 3, 0,  9, i8086AndTestOrXorAccumulator,cmds);  // test Reg/Mem-2Immed



        
        

  //String Manipulation
  /*                  Name   OpC Sz Mod Clk   EmuFkt       cmds                  */
  i8086AddCommand("movsb", 164, 1, 0, 18, i8086Movs, cmds);//movs byte
  i8086AddCommand("movsw", 165, 1, 0, 18, i8086Movs, cmds);//movs word
  i8086AddCommand("cmpsb", 166, 1, 0, 22, i8086Cmps, cmds);//subtrahiert es:di von ds:si und setzt Flags danach
  i8086AddCommand("cmpsw", 167, 1, 0, 22, i8086Cmps, cmds);//subtrahiert es:di von ds:si und setzt Flags danach
  i8086AddCommand("stosb", 170, 1, 0, 11, i8086Stos, cmds);//mov AL nach es:di
  i8086AddCommand("stosw", 171, 1, 0, 11, i8086Stos, cmds);//mov AX nach es:di
  i8086AddCommand("lodsb", 172, 1, 0, 12, i8086Lods, cmds);//mov ds:si nach AL
  i8086AddCommand("lodsw", 173, 1, 0, 16, i8086Lods, cmds);//mov ds:si nach AX
  i8086AddCommand("scasb", 174, 1, 0, 15, i8086Scas, cmds);//subtrahiert es:di von AL und setzt Flags danach
  i8086AddCommand("scasw", 175, 1, 0, 15, i8086Scas, cmds);//subtrahiert es:di von AX und setzt Flags danach
  i8086AddCommand("repne", 242, 1, 0, 2, i8086Rep, cmds);//repnz repne
  i8086AddCommand("repe", 243, 1, 0, 2, i8086Rep, cmds);//rep repz repe

  //control
  /*               Name    OpC Sz Mod Clk   EmuFkt  cmds                  */
  i8086AddCommand("hlt", 244, 1, 0, 2, i8086Halt, cmds);		//hlt
  i8086AddCommand("clc", 248, 1, 0, 2, i8086ClearCary, cmds);
  i8086AddCommand("cmc", 245, 1, 0, 2, i8086ComplementCarry, cmds);
  i8086AddCommand("stc", 249, 1, 0, 2, i8086SetCarry, cmds);
  i8086AddCommand("cld", 252, 1, 0, 2, i8086ClearDirection, cmds);
  i8086AddCommand("std", 253, 1, 0, 2, i8086SetDirection, cmds);
  i8086AddCommand("cli", 250, 1, 0, 2, i8086ClearInt, cmds);
  i8086AddCommand("sti", 251, 1, 0, 2, i8086SetInt, cmds);
  i8086AddCommand("wait", 155, 1, 0, 4, i8086Wait, cmds);
  for (i=216;i<=223;i++)
  {i8086AddCommand("esc", i, 2,  1, 2, i8086Escape, cmds);}
  i8086AddCommand("lock", 240, 1, 0, 2, i8086Lock, cmds);
  i8086AddCommand("nop", 144, 1, 0, 3, i8086Nop, cmds);
  //Segment Overrides
  i8086AddCommand("cs", 0x2E, 1, 0, 3, i8086SegmentOverride, cmds);
  i8086AddCommand("ds", 0x3E, 1, 0, 3, i8086SegmentOverride, cmds);
  i8086AddCommand("es", 0x26, 1, 0, 3, i8086SegmentOverride, cmds);
  i8086AddCommand("ss", 0x36, 1, 0, 3, i8086SegmentOverride, cmds);

  //control transfer
  /*               Name    OpC Sz Mod Clk   EmuFkt  cmds                  */
//  i8086AddCommand("jmp", 233, 3, 0, 18, i8086UncondJumpDS, cmds);
//  i8086AddCommand("jmp", 235, 2, 0, 18, i8086UncondJumpDSS, cmds);
//  i8086AddCommand("jmp", 255, 2, 1, 18, i8086UncondJumpIS, cmds);

  i8086AddCommand("int", 205, 2, 0, 51, i8086Interrupt, cmds);
  i8086AddCommand("int3", 204, 1, 0, 52, i8086Interrupt, cmds); //int 3
  i8086AddCommand("into", 206, 1, 0, 53, i8086Interrupt, cmds); //int on overflow
  i8086AddCommand("iret", 207, 1, 0, 32, i8086InterruptReturn, cmds);
  i8086AddCommand("ret", 195, 1, 0, 16, i8086ReturnS, cmds);	//ret near
  i8086AddCommand("retf", 0xCB, 1, 0, 26, i8086ReturnIS, cmds);	//ret far
  i8086AddCommand("ret", 0xC2, 3, 0, 20, i8086ReturnImmedS, cmds);	//ret near mit SP Verschiebung
  i8086AddCommand("retf", 0xCA, 3, 0, 25, i8086ReturnImmedIS, cmds);	//ret far mit SP Verschiebung
  i8086AddCommand("call", 232, 3, 0, 19, i8086UncondJumpDS, cmds);
  i8086AddCommand("jmp", 233, 3, 0, 15, i8086UncondJumpDS, cmds);
  i8086AddCommand("jmp", 235, 2, 0, 15, i8086UncondJumpDSS, cmds);

  i8086AddCommand("jmp", 0xEA, 5, 0, 15, i8086UncondJumpDIS, cmds);
  i8086AddCommand("call", 0x9A, 5, 0, 28, i8086UncondJumpDIS, cmds);

/*  i8086AddCommand("jmp", 255, 2, 1, 11, i8086UncondJumpIS, cmds); *** ersetzt durch i8086WrapJmpPushInc *** */

  i8086AddCommand("loop", 226, 2, 0, 18, i8086CondJump, cmds);
  i8086AddCommand("jcxz", 227, 2, 0, 18, i8086CondJump, cmds);
  i8086AddCommand("loopne", 224, 2, 0, 18, i8086CondJump, cmds);
  i8086AddCommand("loope", 225, 2, 0, 18, i8086CondJump, cmds);

  i8086AddCommand("jo", 112, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jno", 113, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jb", 114, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jnb", 115, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("je", 116, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jne", 117, 2, 0, 16, i8086CondJump, cmds); /* muesste eigentlich jnz sein */
  i8086AddCommand("jna", 118, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("ja", 119, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("js", 120, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jns", 121, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jp", 122, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jnp", 123, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jl", 124, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jnl", 125, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jle", 126, 2, 0, 16, i8086CondJump, cmds);
  i8086AddCommand("jnle", 127, 2, 0, 16, i8086CondJump, cmds);
}
