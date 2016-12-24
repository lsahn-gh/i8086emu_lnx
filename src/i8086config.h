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
/* Funktionen zum Lesen und Schreiben von           */
/* Config-Files.                                    */
/*                                                  */
/****************************************************/

#ifndef _i8086CONFIG_H_
#define _i8086CONFIG_H_

#include <ctype.h>
#include <stdlib.h>


/* Max. Laenge eines Konfigurationswertes */
#define i8086_CFG_MAX_VALUE_LEN 256
#define CONFIG_FILE "emu.cfg"

/* Wandelt str in Grossbuchstaben um. */
#define strup(str) {int i=0; \
                    while (str[i]!='\0') \
                    {str[i]=toupper(str[i]); i++;} \
                   }
FILE *i8086OpenConfig(const char *filename);
int i8086ConfigExists(const char *filename);
int i8086ReadStrConfig(char *buf, const char *filename, const char *name);
void i8086WriteStrConfig(const char *filename, const char *name, const char *value);
int i8086ReadHexConfig(const char *filename, const char *name, int defValue);
int i8086ReadDecConfig(const char *filename, const char *name, int defValue);
int i8086ReadBoolConfig(const char *filename, const char *name, int defValue);
int i8086NameExistsConfig(const char *filename, const char *name);
#endif /* _i8086CONFIG_H_ */
