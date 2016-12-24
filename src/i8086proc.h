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
/*                                                  */
/*                                                  */
/****************************************************/

#ifndef _i8086PROC_H_
#define _i8086PROC_H_

#include <sys/time.h>
#include "i8086proctypes.h"

/* Variablen */
i8086core *core;
i8086command **commands;

/* Funktionen */

/* Register sezten. w=1 -> 16b, w=0 -> 8b. siehe 231456.pdf Seite 30 */
void i8086SetRegister(i8086core *core, int reg, i8086W w, unsigned short value);
/* Register lesen. w=1 -> 16b, w=0 -> 8b. siehe 231456.pdf Seite 30 */
i8086SingleRegister* i8086GetRegister(i8086core *core, i8086W w, int reg);
void i8086GetRegisterRef(i8086SingleRegister *buf, i8086core *core, i8086W w, int reg);
/* Segmentregister setzen */
void i8086SetSegRegister(i8086core *core, int reg, unsigned short value);
/* Segmentregister lesen */
i8086SingleRegister* i8086GetSegRegister(i8086core *core, int reg, int convert);
unsigned int i8086GetSegRegister_fast(i8086core *core, int reg, int convert);

/* Core und command-Array initialisieren */
void i8086init();
void i8086close();
void i8086reset();
/* com-Datei in Speicher laden*/
int i8086loadBinFile(i8086core* core, const char* filename, unsigned int beginAdr);

/* Befehl in pc ausführen                       */
/* cmds referenziert auf die Emulatorfunktionen */
int i8086execCommand(i8086core *core, i8086command **cmds);

/* Fuegt cmd in cmds an Position opcode ein */
//void i8086addCommand(i8086command **cmds, i8086command *cmd, unsigned char opcode);

#endif /* _i8086PROC_H_ */
