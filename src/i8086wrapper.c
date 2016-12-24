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
/* Wrapper-Funktionen für mehrfach belegte OpCodes. */
/*                                                  */
/****************************************************/

#include "i8086datatrans.h"
#include "i8086arithmetic.h"
#include "i8086controltrans.h"
#include "i8086logic.h"
#include "i8086util.h"

/* OpCode 255 */
void i8086WrapJmpPushInc(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char c = getBitSnipped(para.b[0], 5, 3); /* Bit 5-3 (00xxx000) */

  if (c==6) /* 110b */
    i8086PushMem(core, opcode, para, data);
  else
  if (c==0)
    i8086IncDecRegMem(core, opcode, para, data);
  else
  if (c==4 || c==2 || c==3 || c==5) /* 100b || 010b */
    i8086UncondJumpIS(core, opcode, para, data);
}

/* Opcode 128-131	*/
void i8086WrapAddAdcSubSsbCmpAndOrXor(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char c = getBitSnipped(para.b[0], 5, 3); /* Bit 5-3 (00xxx000) */

 	if(c==1 || c==4 || c==6) /* 110b */
	{
		i8086AndTestOrXorImmed(core, opcode,para, data);
	}
	else
		i8086AddImmed(core, opcode,para, data);
}

/*Opcode 246-247*/
void i8086WrapNegNotMulDivTest(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	switch (getBitSnipped(para.b[0], 5, 3)) /* Bit 5-3 (00xxx000) */
	{
                case 0:
                        i8086AndTestOrXorImmed(core, opcode,para, data);
                        break;
		case 2:	//010b
			i8086not(core, opcode,para, data);
			break;
		case 3:
			i8086Negate(core, opcode,para, data);
			break;
		case 4:
			i8086Multiplicate(core, opcode,para, data);
			break;
		case 5:
			i8086Multiplicate(core, opcode,para, data);
			break;
		case 6:
			i8086Divide(core, opcode,para, data);
			break;
		case 7:
			i8086Divide(core, opcode,para, data);
			break;
	}
}

/* Funktion ur Auswahl der verschiedenen Funktionen da sie den gleichen Opcode besitzen */
void i8086ShlSalShrRolRorRclRcr(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char wData;
  unsigned char res;

  wData = opcode & i8086_BIT_0;                     /* 1=16bit, 0=8bit           */

  res=getBitSnipped(para.b[0],5,3); /* Mod heraus ziehen (**XXX***) */

  if(res==4 || res==5 || res==7)
       i8086ShlShrSar(core, opcode, para, data);
  else if(res==0 || res==1 || res==2 || res==3)
       i8086RolRorRclRcr(core, opcode, para, data);
}

