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
/*                                                  */
/*                                                  */
/* Zeichen Funktionen für Oberflaeche.              */
/*                                                  */
/****************************************************/

#ifndef _i8086_GUI_PAINT_H_
#define _i8086_GUI_PAINT_H_

#include <gtk/gtk.h>
#include "i8086error.h"

#ifdef _WIN32
 #define i8086_GUI_MEM_FONT_BOLD "-*-fixed-bold-r-normal-*-*-120-*-*-m-*-*-*"
 #define i8086_GUI_MEM_FONT "-*-fixed-*-r-normal-*-*-120-*-*-m-*-*-*"
 #define i8086_GUI_DISASM_FONT_BOLD "-*-fixed-bold-r-normal-*-*-120-*-*-m-*-*-*"
#else
 #define i8086_GUI_MEM_FONT_BOLD "-*-fixed-bold-r-normal-*-*-120-*-*-*-*-*-*"
 #define i8086_GUI_MEM_FONT "-*-fixed-*-r-normal-*-*-120-*-*-*-*-*-*"
 #define i8086_GUI_DISASM_FONT_BOLD "-*-fixed-bold-r-normal-*-*-120-*-*-*-*-*-*"
#endif

#define i8086_GUI_DEFAULT_FONT_PRIM "-*-*-*-*-*-*-*-120-*-*-m-*-*-*"
#define i8086_GUI_DEFAULT_FONT_SEC "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

/* Makro */
/* Laed einen Font. Konnte der Font nicht geladen werden   */
/* wird zuerst versucht den primaeren Default-Font zu      */
/* laden. Konnte auch der prim. Def.-Font nicht geladen    */
/* werden wird er sekundaere versucht. Scheitern alle drei */
/* Versuche wird NULL in buf zurueck gegeben.              */
/*  buf: *fontBold - Puffer für geladenen Font             */
/*  fontName: *char - Fontname, XLFD kodiert               */
#define i8086_GET_GTKFONT(buf, fontName) { buf=gdk_font_load(fontName); \
                                           if (buf==NULL){ i8086warning(i8086_ERR_LOAD_FONT); buf=gdk_font_load(i8086_GUI_DEFAULT_FONT_PRIM); } \
                                           if (buf==NULL){ i8086warning(i8086_ERR_LOAD_PRIM_FONT); buf=gdk_font_load(i8086_GUI_DEFAULT_FONT_SEC); } \
                                           if (buf==NULL){ i8086warning(i8086_ERR_LOAD_SEC_FONT); } \
                                         }

/* Makro */
/* Setzt die RGB-Werte fuer eine GdkColor-Variable */
/* buf: GdkColor */
#define i8086_SET_COLOR(buf, r, g, b) {buf.red = r; buf.green = g; buf.blue = b;}
                                         
typedef struct codeView
{
  unsigned int startAdr;
  unsigned int endAdr;
}codeView;

void paint_leds (GtkWidget *widget, GdkEventExpose *event);
//seg Inalt //welches Segment(0-7)
void paint_display_segment(GtkWidget *widget, char bits,char seg);
void paint_display(GtkWidget *widget, GdkEventExpose *event);
guint showStatusbarMessage(const gchar  *text);
void paint_mem(GtkWidget *widget, GdkEventExpose *event, i8086core *core, unsigned int startAdr);
void paint_disasm(GtkWidget *widget, GdkEventExpose *event, i8086core *core, i8086command **cmds, char reset);
void on_memviewer_size_allocate (GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data);
void on_disasmviewer_size_allocate (GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data);
void on_memspin_change_value(GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data);
void on_memtextfield_change_value(GtkWidget  *widget, GdkRectangle *allocation, gpointer userdata);
void changeMemReg (GtkWidget *widget,gpointer rm);
void setBreakpoint(GtkWindow  *widget,gpointer callback_data);
void updateTree();
GtkWidget *bpadrtextbox;
GtkListStore *store;

void about_win(GtkWindow  *widget);
void help_win(GtkWindow  *widget);


#endif /* _i8086GUI_PAINT_H_ */
