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
/* Autor: CST                                       */
/* Version: 1.0                                     */
/* ** => nicht beendet                              */
/*                                                  */
/* String-Manipulationsfunktionen		            */
/*                                                  */
/****************************************************/
#ifndef _i8086STRINGS_H_
#define _i8086STRINGS_H_

#include "i8086proc.h"

void i8086Rep(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Repeat
void i8086Movs(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Movs
void i8086Cmps(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Cmps
void i8086Scas(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Scas
void i8086Lods(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Lods
void i8086Stos(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);//Stos
#endif /* _i8086STRINGS_H_ */
