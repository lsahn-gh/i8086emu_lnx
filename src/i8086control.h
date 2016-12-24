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
/* Prozessorkontrollfunktionen	                    */
/*                                                  */
/****************************************************/

#ifndef _i8086CONTR_H_
#define _i8086CONTR_H_

#include "i8086proc.h"

void i8086Halt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086ClearCary(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086ComplementCarry(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086SetCarry(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);

void i8086ClearDirection(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086SetDirection(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086SetInt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086ClearInt(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086Wait(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086Lock(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086Escape(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086Nop(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);
void i8086SegmentOverride(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);

#endif /* _i8086CONTR_H_ */
