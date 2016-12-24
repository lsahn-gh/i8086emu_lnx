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
/* Prozessorkontrollfunktionen			    */
/*                                                  */
/****************************************************/

#include "i8086proc.h"
#include <unistd.h>


void i8086Halt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//_exit(1);
	//befehl nicht implementiert
}

//die Funktion folgender Befehle sollte sich aus dem Namen erkennen lassen

void i8086ClearCary(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags & (~i8086_FLAG_CF);
}
void i8086ComplementCarry(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags ^ (i8086_FLAG_CF);
}

void i8086SetCarry(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags | i8086_FLAG_CF;
}

void i8086ClearDirection(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags & (~i8086_FLAG_DF);
}

void i8086SetDirection(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags | i8086_FLAG_DF;
}

void i8086SetInt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags | i8086_FLAG_IF;
}

void i8086ClearInt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	core->flags=core->flags & (~i8086_FLAG_IF);
}

void i8086Wait(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//befehl nich implementiert, da kein coprozessor
}

void i8086Lock(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//befehl nich implementiert
}
void i8086Escape(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//befehl nich implementiert
}

void i8086Nop(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	//befehl wartet 3 Takte und das wars
}


//setzt den die entsprechende Segmentnummer für den angeforderten Segmentoverride
void i8086SegmentOverride(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	switch (opcode)
	{
		case 0x2E:
			core->miscData.segoverride=i8086_REG_CS;
			break;
		case 0x26:
			core->miscData.segoverride=i8086_REG_ES;
			break;
		case 0x36:
			core->miscData.segoverride=i8086_REG_SS;
			break;
		case 0x3E:
			core->miscData.segoverride=i8086_REG_DS;
			break;
	}
}
