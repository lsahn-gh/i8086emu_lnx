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
/* Autor: RD                                        */
/* Version: 1.0                                     */
/*                                                  */
/* Sprungfunktionen etc.			    */
/*                                                  */
/****************************************************/

#include "i8086proc.h"
#include "i8086util.h"
#include "i8086emufuncs.h"

void i8086UncondJumpDS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
//jmp/call direct im segment
{
	//i8086Parameter p=para;
	if (opcode==232)	//call
	{
		push(core,core->pc);
	}

	core->pc+=joinBytes(para.b[1],para.b[0]);
}

void i8086UncondJumpDIS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
//jmp/call direct Intersegment
{
	//i8086Parameter p=para;
	if (opcode==154)	//call
	{
		push(core,i8086GetSegRegister_fast(core,i8086_REG_CS,0));
		push(core,core->pc);
	}

	core->pc=joinBytes(para.b[1],para.b[0]);
	i8086SetSegRegister(core,i8086_REG_CS,joinBytes(para.b[3],para.b[2]));
}

//löst Interrupt n aus
void i8086callInterrupt(i8086core *core, unsigned char n)
{
	i8086SingleRegister stackreg;

	i8086GetRegisterRef(&stackreg, core, 1, i8086_REG_SP); //stackregister

	stackreg.x-=2							;//dekrementiere stackpointer um 2
	memWriteWord(core,stackreg.x,core->flags,i8086_REG_SS);		//flags pushen
	core->flags=core->flags & (~i8086_FLAG_IF);	//clear Interrupt-Flag
	core->flags=core->flags & (~i8086_FLAG_TF);	//clear Trap-Flag
	//hier sollte CS gepusht werden
	stackreg.x-=2							;//dekrementiere stackpointer um 2
	memWriteWord(core,stackreg.x,i8086GetSegRegister_fast(core,i8086_REG_CS,0),i8086_REG_SS);		//CS pushen
	//hier sollte CS gesetzt werden
	stackreg.x-=2;							//dekrementiere stackpointer um 2
	memWriteWord(core,stackreg.x,core->pc,i8086_REG_SS);		//pc pushen


	i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x); //stackpointer schreiben

	//free(stackreg);

	core->pc=memReadWord(core,n*4,i8086_SEGMENT0);
	i8086SetSegRegister(core,i8086_REG_CS,memReadWord(core,n*4+2,i8086_SEGMENT0));
}

// int und int3
void i8086Interrupt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	if (opcode==204)
	{
		i8086callInterrupt(core,3);
	}
	else
	{
		i8086callInterrupt(core,para.b[0]);
	}
}

// into
void i8086InterruptOverflow(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	if (core->flags & i8086_FLAG_OF)
		i8086callInterrupt(core,4);
}

// iret
void i8086InterruptReturn(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister stackreg;

	i8086GetRegisterRef(&stackreg, core, 1, i8086_REG_SP); //stackregister

	core->pc=memReadWord(core,stackreg.x,i8086_REG_SS);		//pc poppen
	stackreg.x+=2;							//inkrementiere stackpointer um 2

	//hier sollte CS gepopt werden
	i8086SetSegRegister(core,i8086_REG_CS,memReadWord(core,stackreg.x,i8086_REG_SS));	//CS poppen
	stackreg.x+=2;							//inkrementiere stackpointer um 2

	core->flags=memReadWord(core,stackreg.x,i8086_REG_SS) | i8086_CPU_SIG; //obere 4bits immer 1
	stackreg.x+=2;							//inkrementiere stackpointer um 2

	i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x); //stackpointer schreiben

	//free(stackreg);
}

//return im gleichem Segment
void i8086ReturnS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->pc=pop(core);
}

//return im gleichem Segment, Konstante wird zu SP addiert
void i8086ReturnImmedS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->reg.x[i8086_AR_SP]+=joinBytes(para.b[1],para.b[0]);
	core->pc=pop(core);
}

//return InterSegment
void i8086ReturnIS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->pc=pop(core);
	i8086SetSegRegister(core,i8086_REG_CS,pop(core));
}

//return InterSegment, Konstante wird zu SP addiert
void i8086ReturnImmedIS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->reg.x[i8086_AR_SP]+=joinBytes(para.b[1],para.b[0]);
	core->pc=pop(core);
	i8086SetSegRegister(core,i8086_REG_CS,pop(core));
}

//jmp short (direkter Sprung gleichem Segment)
void i8086UncondJumpDSS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//i8086Parameter p=para;
	core->pc+=(signed char)para.b[0];
}

//jmp/call indirekt im Segment/Intersegment
void i8086UncondJumpIS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i808616BitAdr adr;
	char reg;
	i8086SingleRegister sr;
	char subcmd=getBitSnipped(para.b[0], 5, 3);

	reg=para.b[0]&7;

	if (subcmd==3) //call interseg.
		push(core,i8086GetSegRegister_fast(core,i8086_REG_CS,0));

	if (subcmd==2 || subcmd==3)	//call
		push(core,core->pc);


	if ((para.b[0] & 192) == 192) //Sprungadress in Register?
	{
  	i8086GetRegisterRef(&sr,core,1,reg);
  	core->pc=sr.x;
		//core->pc=i8086GetRegister(core,1,reg)->x;
	}
	else
	{	//Sprungadresse im Speicher!
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));
		core->pc=memReadWord(core,adr,i8086_REG_DS);

		if (subcmd==3 || subcmd==5) //call/jmp interseg.
			i8086SetSegRegister(core,i8086_REG_CS,memReadWord(core,adr+2,i8086_REG_DS));
	}
}

//bedingte Sprünge und Loops
void i8086CondJump(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

//Opcodes
//112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 1m27
//224 225 226 227

	i8086Parameter p=para;
	switch (opcode)
	{
		case 116:	//JE/JZ
			if (core->flags & i8086_FLAG_ZF) core->pc+=(signed char)p.b[0];
			break;

		case 124:	//JL/JNGE
			if (((core->flags & i8086_FLAG_SF) << 4) != (core->flags & i8086_FLAG_OF)) core->pc+=(signed char)p.b[0];
			break;

		case 126:	//JLE/JNG
			if ((core->flags & i8086_FLAG_ZF) || (((core->flags & i8086_FLAG_SF) << 4) != (core->flags & i8086_FLAG_OF))) core->pc+=(signed char)p.b[0];
			break;

		case 114:	//JB/JNAE
			if (core->flags & i8086_FLAG_CF) core->pc+=(signed char)p.b[0];
			break;

		case 118:	//JBE/JNA
			if ((core->flags & i8086_FLAG_CF) || (core->flags & i8086_FLAG_ZF)) core->pc+=(signed char)p.b[0];
			break;

		case 122:	//JP/JPE
			if (core->flags & i8086_FLAG_PF) core->pc+=(signed char)p.b[0];
			break;

		case 112:	//JO
			if (core->flags & i8086_FLAG_OF) core->pc+=(signed char)p.b[0];
			break;

		case 120:	//JS
			if (core->flags & i8086_FLAG_SF) core->pc+=(signed char)p.b[0];
			break;

		case 117:	//JNE/JNZ
			if (!(core->flags & i8086_FLAG_ZF)) core->pc+=(signed char)p.b[0];
			break;

		case 125:	//JNL/JGE
			if (((core->flags & i8086_FLAG_SF) << 4) == (core->flags & i8086_FLAG_OF)) core->pc+=(signed char)p.b[0];
			break;

		case 127:	//JNLE/JG
			if ( (!(core->flags & i8086_FLAG_ZF)) && (((core->flags & i8086_FLAG_SF) << 4) == (core->flags & i8086_FLAG_OF)) ) core->pc+=(signed char)p.b[0];
			break;

		case 115:	//JNB/JAE
			if (!(core->flags & i8086_FLAG_CF)) core->pc+=(signed char)p.b[0];
			break;

		case 119:	//JNBE/JA
			if ( (!(core->flags & i8086_FLAG_CF)) && (!(core->flags & i8086_FLAG_ZF)) ) core->pc+=(signed char)p.b[0];
			break;

		case 123:	//JNP/JPO
			if ( !(core->flags & i8086_FLAG_PF) ) core->pc+=(signed char)p.b[0];
			break;

		case 113:	//JNO
			if ( !(core->flags & i8086_FLAG_OF) ) core->pc+=(signed char)p.b[0];
			break;

		case 121:	//JNS
			if ( !(core->flags & i8086_FLAG_SF) ) core->pc+=(signed char)p.b[0];
			break;

		case 226:	//LOOP
			core->reg.x[i8086_AR_CX]--;
			if (core->reg.x[i8086_AR_CX]) core->pc+=(signed char)p.b[0];
			break;

		case 225:	//LOOPZ/LOOPE
			core->reg.x[i8086_AR_CX]--;
			if ((core->flags & i8086_FLAG_ZF) && (core->reg.x[i8086_AR_CX]))
				core->pc+=(signed char)p.b[0];
			break;

		case 224:	//LOOPNZ/LOOPNe
			core->reg.x[i8086_AR_CX]--;
			if ((!(core->flags & i8086_FLAG_ZF)) && (core->reg.x[i8086_AR_CX]))
				core->pc+=(signed char)p.b[0];
			break;

		case 227:	//JCXZ
			if (!(core->reg.x[i8086_AR_CX])) core->pc+=(signed char)p.b[0];
			break;
	}
}
