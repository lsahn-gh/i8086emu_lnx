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
/* Hilfsfunktionen fuer die Oberflaeche.            */
/*                                                  */
/****************************************************/

#ifndef _i8086_GUI_UTIL_H_
#define _i8086_GUI_UTIL_H_

int i8086getFontHeight(GtkWidget *widget);
int i8086getTextWidth(GtkWidget *widget, const char *text);
void i8086guiClearText(GtkWidget *text);
unsigned int *breakpoints;
unsigned char bpelements;


unsigned char addBp(unsigned int bpoint);
unsigned char delBp(unsigned int bpoint);
unsigned char isBreakpoint(unsigned int adr);
unsigned char isValidHex(char *str);


/*folgende Funktionen entstammen unveraendert dem Sourcecode für X-Chat 2.0.6 (c) 1998-2003 Peter Zelezny*/
/*http://www.xchat.org*/
/*Diese Zeilen stehen unter der GPL*/
int get_mhz (void);
char * get_cpu_str (void);
void get_cpu_info (int *mhz, int *cpus);
int waitline (int sok, char *buf, int bufsize);
#endif /* _i8086GUI_UTIL_H_ */
