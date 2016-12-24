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

#ifndef _i8086_GUI_ERROR_H_
#define _i8086_GUI_ERROR_H_

/* Makros fuer Fkts */
#define i8086guiError(parent, msg1, msg2) i8086guiErrorEx(parent, __FILE__, __LINE__, msg1, msg2)
#define i8086guiMessage(parent, msg1, msg2, mode)   { char *msg; \
  msg = (char*)calloc(strlen(msg1)+strlen("\n")+strlen(msg2), sizeof(char)); \
  strcat(msg, msg1); \
  if (strlen(msg2)>0) \
  { strcat(msg, "\n"); strcat(msg, msg2); } \
  i8086guiShowMessage(GTK_WINDOW(parent), msg,  mode); free (msg); }

void i8086guiErrorEx(GtkWidget *parent, const char *filename, int linenum, const char *msg1, const char *msg2);
void i8086guiShowMessage(GtkWindow *parent, const char *msg, int mode);

#endif /* _i8086_GUI_ERROR_H_ */
