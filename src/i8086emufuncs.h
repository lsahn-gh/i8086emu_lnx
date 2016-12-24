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
/* Autor: JMH                                       */
/* Version: 1.0                                     */
/* ** => nicht beendet                              */
/*                                                  */
/* Hilfsfunktionen für die Emulatorfunktionen       */
/*                                                  */
/****************************************************/
#ifndef _i8086EMUFUNCS_H_
#define _i8086EMUFUNCS_H_

#include "i8086proc.h"

/* Erstellt eine neue i8086command*-Struktur */
i8086command* i8086AllocCommnand(char name[i8086_OPC_NAME_LEN], unsigned char opcode, 
                                 unsigned char size, unsigned char hasMod, unsigned char clocks,
                                 void (*func)(i8086core *core, unsigned char command, i8086Parameter para, i8086Parameter data)
                                );
                                
/* Erstellt eine neue i8086command*-Struktur   */
/* und fuegt sie in das Kommando-Array cmds ein */
i8086command* i8086AddCommnand(char name[i8086_OPC_NAME_LEN], unsigned char opcode, 
                               unsigned char size, unsigned char hasMod, unsigned char clocks,
                               void (*func)(i8086core *core, unsigned char command, i8086Parameter para, i8086Parameter data),
                               i8086command **cmds
                              );

/**/
void i8086InitCommands(i8086command **cmds);

i808616BitAdr decodeMemAdr(i8086core *core, char modrm, signed short disp);
unsigned char getAdditionalCmdLength(i8086core *core, unsigned char opcode, char modrm);
void memWriteByte(i8086core *core, i808616BitAdr adr, char val,int reg);
void memWriteWord(i8086core *core, i808616BitAdr adr, unsigned short val,int reg);
char memReadByte(i8086core *core, i808616BitAdr adr,int reg);
unsigned short memReadWord(i8086core *core, i808616BitAdr adr,int reg);
//unsigned short joinBytes(unsigned char a,unsigned char b); //f?gt 2 bytes zu einem word zusammen, a high, b low
void correctArithmeticFlags(i8086core *core, unsigned short i,unsigned char w);
/*liest oder schreibt ein BYTE auf einen INPUT oder OUTPUT-Port*/
unsigned char portOpByte(i8086core *core,unsigned char output,unsigned char data, unsigned short portNum, unsigned char write);
/*liest oder schreibt ein WORD auf einen INPUT oder OUTPUT-Port*/
unsigned short portOpWord(i8086core *core,unsigned char output,unsigned short data, unsigned short portNum, unsigned char write);

void correctOvCaFlagafterAddSub(i8086core *core, unsigned short a, unsigned short b, unsigned short c, unsigned char w, unsigned char addsub);
void memWrite(i8086core *core, i808616BitAdr adr, unsigned short val, unsigned char w,int reg); //schreibt 16/8bit (abhängig von w) aus dem Speicher
unsigned short memRead(i8086core *core, i808616BitAdr adr, unsigned char w,int reg); //liest 16/8bit (abhängig von w) aus dem Speicher
void push(i8086core *core, unsigned short x);
unsigned short pop(i8086core *core);

#endif /* _i8086EMUFUNCS_H_ */
