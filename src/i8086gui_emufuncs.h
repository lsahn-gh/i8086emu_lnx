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
/****************************************************/

#ifndef _i8086_GUI_EMUFUNCS_H_
#define _i8086_GUI_EMUFUNCS_H_

#include <gtk/gtk.h>
#include <glib.h>
#include "i8086proc.h"

#define i8086_GUI_REG_FONT "Courier Bold 10"

//char shutdown;
//char canRun;

void i8086guiInitEmu();
void i8086guiCloseEmu();
void i8086guiNextCommand(GtkWidget *button, gpointer data);
void i8086guiStepOver(GtkWidget *button, gpointer data);
void i8086guiRun(GtkWidget *button, i808616BitAdr data);
void i8086guiStop(GtkWidget *button, gpointer data);
void i8086guiPaintAllRegister(GtkWidget *widget);
void i8086guiPaintAllFlags(GtkWidget *widget);
void i8086guiPaintAll();
void i8086guiInitMaps();
void i8086gui_keyboard_button_pressed (GtkButton *button, gpointer user_data);
char i8086gui_keyboard_button_released (GtkButton *button, gpointer user_data);
void i8086gui_keyboard_button_clicked (GtkButton *button, gpointer user_data);
void i8086gui_cpu_reset (GtkButton *button, gpointer user_data);
void i8086gui_on_switch_toggled (GtkButton *button, unsigned char user_data);
void i8086gui_adjust_update_interval(GtkRange *range, gpointer user_data);
void i8086event_addbp(GtkWidget *button, gpointer user_data);
void i8086event_delbp(GtkWidget *button, gpointer user_data);
void i8086event_rowchanged(GtkTreeView *treeview, gpointer data);
void i8086event_closeDialog(GtkWidget *button, gpointer user_data);
char *stringcat(const char *s, ...);
void i8086guiLoadBinFile(const char *filename, unsigned int adr);
void i8086guiLoadConfig(const char *filename);

char *selectedRow;//akutell ausgewaehlte reihe im breakpoints dialogfenster

#endif /* _i8086_GUI_EMUFUNCS_H_ */
