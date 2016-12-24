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
/* kleine Hilfsfunktionen		            */
/*                                                  */
/****************************************************/

#ifndef _i8086UTIL_H_
#define _i8086UTIL_H_

#include "i8086proc.h"

#define i8068_TPI 0.2     /* 8086 -> 5MHz -> Benoetigte Zeit(micSec) für einen Befehl. */

//fuegt 2 bytes zu einem word zusammen, a high, b low
#define joinBytes(a, b) ((unsigned short)((a<<8)|b))

//schaut ob Segmentoverride angefordert wurde und setzt ihn ggf. um
#define doSegmentOverride(core,seg) if (seg==i8086_REG_DS && core->miscData.segoverride >  -1) seg=core->miscData.segoverride;

//unsigned char getBitSnipped(unsigned char b,unsigned char pos, unsigned char length);
/* Autor: RD */
/* Liefert length bits ab bit pos aus b.  */
/* b, pos, length, return : unsigned char */
/* Bsp: getBitSnipped(b, 5, 3) liefert    */
/*      die bits die mit x markiert sind  */
/*      Pos: 76543210                     */
/*             |->                        */
/*           00xxx000                     */
#define getBitSnipped(b, pos, length) ((unsigned char)(((unsigned char)((unsigned char)(b) << (7-(unsigned char)(pos)))) >> (7-((unsigned char)(length)-1))))

#define getBitSnippedInt(b, pos, length) ((unsigned int)(((unsigned int)((unsigned int)(b) << (31-(unsigned int)(pos)))) >> (31-((unsigned int)(length)-1))))

//unsigned short swapbytes(unsigned short i);
/* vertauscht die oberen 8bit mit den unteren 8bit von i*/
/* i, return : unsigned short */
#define swapbytes(i) ((unsigned short)(((unsigned short)i<<8) | ((unsigned short)i>>8)))

char *stringAdd(char *s1, ...);

/* Berechnet die Zeitdiff. zw 8086- und emu-Ausfuehrungsgeschwindigkeit */
/* und setzt den emu gegebenenfalls in den sleep-modus bis die          */
/* Ausfuehrungsgeschwindigkeit wieder synchron sind.                    */
void cmdWait(unsigned int clocks, unsigned long usedTime);
void delay(long microsec);
int copyFile(char *src, char *dest);

#endif /* _i8086UTIL_H_ */
