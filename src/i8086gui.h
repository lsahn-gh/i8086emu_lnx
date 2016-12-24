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
/* Autor: FB                                        */
/* Version: 1.0                                     */
/* ** => nicht beendet                              */
/*                                                  */
/*	Gui						                                  */
/*                                                  */
/****************************************************/

#ifndef _i8086_GUI_H_
#define _i8086_GUI_H_

#include <gtk/gtk.h>
#include <glib.h>

GtkWidget *window;	//Haupt Fenster
GtkWidget *leds;
GtkWidget *anz;
GtkWidget *flaglabel;
GtkWidget *reglabel;
GtkWidget *statusbar;
GtkWidget *memviewer;
GtkWidget *disasmviewer;
GtkWidget *memSpinButton;
GtkWidget *runbutton;
GtkWidget *tracebutton;
GtkWidget *stepoverbutton;
GtkWidget *runmenuitem;
GtkWidget *tracemenuitem;
GtkWidget *stepovermenuitem;
GtkWidget *memTextField;

void destroy(GtkWidget *widget, GdkEvent *event, gpointer user_data);


#endif /* _i8086_GUI_H_ */
