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
 
/* Autor: JMH                                        */
/* Version: 1.0                                     */
/*                                                  */
/*                                                  */
/* Funktionen zum Malen der Oberflaeche.            */
/*                                                  */
/****************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "i8086proc.h"
#include "i8086error.h"
#include "i8086gui_error.h"
#include "i8086gui.h"
#include "i8086gui_paint.h"
#include "i8086gui_emufuncs.h"
#include "i8086gui_util.h"
#include "i8086util.h"
#include "i8086emufuncs.h"
#include "disasm/disasm.h"
#include "i8086config.h"

unsigned char selectReg=0;	//Zu änderndes Register für Change Reg
GtkWidget *valuetextfield;
GtkWidget *adrtextfield;
gchar *text_buffer;		//Text Buffer für Help

//Autor: FB 
//Led's zeichnen
void paint_leds (GtkWidget *widget, GdkEventExpose *event)
{
  gint i, j;
  unsigned char n,k;
  GdkGC *gc1, *gc2, *gc, *gc3; // "Farbspeicher"
  GdkColor color;
  char bits;

  #define SIZE 11	   //Größe der Rechtecke
  #define SPACING_LED 7  //Sprunggröße
  #define X_OFFSET 8  //Sprunggröße

  bits=core->ports.x[i8086_PORTS];

  // Farben "mischen" und zu weisen
  gc1 = gdk_gc_new (widget->window);
  color.red = 65535;
  color.green = 0;
  color.blue = 0;
  gdk_gc_set_rgb_fg_color (gc1, &color);

  gc2 = gdk_gc_new (widget->window);
  color.red   = 45000;
  color.green = 40000;
  color.blue  = 40000;
  gdk_gc_set_rgb_fg_color (gc2, &color);

  gc3 = gdk_gc_new (widget->window);
  color.red   = 0;
  color.green = 0;
  color.blue  = 0;
  gdk_gc_set_rgb_fg_color (gc3, &color);

  //********** Rahmen zeichnen *************/
  i = X_OFFSET;
  //i=(widget->allocation.width/2)-(SIZE/2);
  j=(widget->allocation.height/2)-(SIZE/2);

  for(n=7;n!=255;n--)
 	{
		gdk_draw_rectangle (widget->window, gc3, TRUE, i, j, SIZE, SIZE);
	  i = SPACING_LED+i+SIZE;
  }

  i= X_OFFSET+1;	 //x-Rand-Koordinate
  k= 128; //um Bits zu testen (10000000 Binär)

  //********** Leds zeichnen ****************/
  for(n=7;n!=255;n--)
 	{
		if ((bits&k)>0)
	    gc = gc1;
	  else
	    gc = gc2;
	 	gdk_draw_rectangle (widget->window, gc, TRUE, i, j+1, SIZE-2, SIZE-2);

	 	i = SPACING_LED+i+SIZE;
  	k=k/2;
  }

  g_object_unref (gc1);//free() ...
  g_object_unref (gc2);
  g_object_unref (gc3);
}

//Autor: FB
//Anzeige zeichnen
																				//seg Inalt //welches Segment(0-7)
void paint_display_segment (GtkWidget *widget, char bits,char seg)
{
  //gint i, j;
  //unsigned char n,k;
  GdkGC *gc1, *gc2, *gc; // "Farbspeicher"
  GdkColor color;
  int xstart,xend,ystart,yend;
  unsigned char width=13;
  unsigned char height=21;

  #define SPACING 23  //Sprunggröße

  // Farben "mischen" und zu weisen
  gc1 = gdk_gc_new (widget->window);
  color.red = 65535;
  color.green = 0;
  color.blue = 0;
  gdk_gc_set_rgb_fg_color (gc1, &color);

  gc2 = gdk_gc_new (widget->window);
  color.red   = 0xFF90;
  color.green = 0xFF90;
  color.blue  = 0xFF90;
  gdk_gc_set_rgb_fg_color (gc2, &color);

  xstart=6;
  ystart=5;
  xend=xstart;
  yend=5+height;

  //********** Anzeige zeichnen ****************/
  xstart+=SPACING*seg;
	xend+=SPACING*seg;
 	  // |
 	  if(bits&32)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
	 	gdk_draw_line (widget->window,gc,xstart, ystart,xend, ystart+height/2-1);
		gdk_draw_line (widget->window,gc,xstart+1,ystart+1,xend+1,ystart+height/2-1-1);
	  if(bits&16)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
	 	gdk_draw_line (widget->window,gc,xstart,ystart+height/2+2,xend,ystart+height);
		gdk_draw_line (widget->window,gc,xstart+1,ystart+height/2+2+1,xend+1,ystart+height-1);
	  // |
	  if(bits&2)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+width,ystart,xend+width,ystart+height/2-1);
	  gdk_draw_line (widget->window,gc,xstart+width-1,ystart+1,xend+width-1,ystart+height/2-1-1);
	  if(bits&4)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+width,ystart+height/2+2,xend+width,ystart+height);
	  gdk_draw_line (widget->window,gc,xstart+width-1,ystart+height/2+2+1,xend+width-1,ystart+height-1);
	  //_-
	  if(bits&1)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+2,ystart-2,xend+width-2,ystart-2);
	  gdk_draw_line (widget->window,gc,xstart+2+1,ystart-2+1,xend+width-2-1,ystart-2+1);
	  if(bits&64)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+2,ystart+height/2,xend+width-2,ystart+height/2);
	  gdk_draw_line (widget->window,gc,xstart+2,ystart+height/2+1,xend+width-2,ystart+height/2+1);
		if(bits&8)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+2,ystart+height+2,xend+width-2,ystart+height+2);
	  gdk_draw_line (widget->window,gc,xstart+2+1,ystart+height+2-1,xend+width-2-1,ystart+height+2-1);
  	//.
		if(bits&128)
 	  	gc=gc1;
 	  else
 	    gc=gc2;
 	  gdk_draw_line (widget->window,gc,xstart+width+2,ystart+height+1,xend+width+4,ystart+height+1);
		gdk_draw_line (widget->window,gc,xstart+width+2,ystart+height+2,xend+width+4,ystart+height+2);



  g_object_unref (gc1);//free() ...
  g_object_unref (gc2);
}

//Autor: FB
void paint_display(GtkWidget *widget, GdkEventExpose *event)
{
  unsigned char i=0;

  for(i=0;i<=7;i++)
  {
     paint_display_segment(anz, core->ports.x[i*2+0x90+i8086_PORTS],7- i);
  }

}

guint showStatusbarMessage(const gchar  *text)
{
 	gint context_id;
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar), "");
	return gtk_statusbar_push (GTK_STATUSBAR (statusbar), GPOINTER_TO_INT (context_id),text);
}

void removeStatusbarMessage(guint id)
{
	//gtk_statusbar_pop( GtkStatusbar *statusbar) id );
}

void paint_mem(GtkWidget *widget, GdkEventExpose *event, i8086core *core, unsigned int startAdr)
{
  char *memstr="", *tmp;
  char numstr[10];
  int i=0, maxWidth=0, cols, j;
  int strHeight=0;
  GdkColor white, black;
  unsigned int IP;
  
  GdkFont *fontBold, *font; // = gdk_font_load(i8086_GUI_MEM_FONT_BOLD);
  i8086_GET_GTKFONT(fontBold, i8086_GUI_MEM_FONT_BOLD);

  font = fontBold;
  //GdkFont *font = gdk_font_load(i8086_GUI_MEM_FONT_BOLD);
  
  if (fontBold==NULL || font==NULL)
  {
    i8086warning(i8086_ERR_LOAD_FONT);
    return; 
  }
  
  i8086_SET_COLOR(white, 65535, 65535, 65535);
  i8086_SET_COLOR(black, 0, 0, 0);

  strHeight=fontBold->ascent+fontBold->descent;
  IP = (core->pc+i8086GetSegRegister_fast(core, i8086_REG_CS, 1));
  
  gtk_text_freeze(GTK_TEXT(widget));
  i8086guiClearText(widget);
  memstr = stringcat("_________ ", NULL);
  
  /* Kopfzeile */
  //while(maxWidth<widget->allocation.width) /* dyn. Breite */
  for (i=0; i<16; i++)
  {
    sprintf(numstr, "%02x ", i);
    tmp = stringcat(memstr, numstr, NULL);
    free(memstr);
    memstr=tmp;
    
    maxWidth=gdk_string_width(fontBold, memstr);
    sprintf(numstr, "%02x ", i+1);
    maxWidth += gdk_string_width(fontBold, numstr)*2;
    //i++;
  }
  cols = i;

  gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, memstr, -1);
  free(memstr);
  
  /* Spalten */
  for (i=0; i<(((widget->allocation.height)/strHeight)-1); i++)
  {
    sprintf(numstr, "\n%04x:%04x", CAST_TO_BASE((startAdr+(i*cols))/0x10000, 17), (i808616BitAdr)(CAST_TO_MEMSIZE(startAdr+(i*cols))));
    gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, numstr, -1);
    
    for (j=0; j<cols; j++)
    {
      sprintf(numstr, "%02x", core->mem[CAST_TO_MEMSIZE(startAdr+(i*cols)+j)]);

      gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, " ", -1);
      if (IP==CAST_TO_MEMSIZE(startAdr+(i*cols)+j)) /* aktuellen OpCode sichtbar machen */
        gtk_text_insert(GTK_TEXT(widget), fontBold, &white, &black, numstr, -1);
      else
        gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, numstr, -1);
    }

   gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, " ", -1);

    for (j=0; j<cols; j++)
    {
      if (isgraph(core->mem[CAST_TO_MEMSIZE(startAdr+(i*cols)+j)]))
        sprintf(numstr, "%c", core->mem[CAST_TO_MEMSIZE(startAdr+(i*cols)+j)]);
      else
        sprintf(numstr, ".");

      gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, numstr, -1);
    }
  }
  
  gtk_text_thaw(GTK_TEXT(widget));
}

void paint_disasm(GtkWidget *widget, GdkEventExpose *event, i8086core *core, i8086command **cmds, char reset)
{
  unsigned char rowstr[40], cmddata[20];
  unsigned char opcode, cmdsize;
  unsigned short strHeight=0, rowCount=0, i, j;
  unsigned int IP, startAdr, read=0;
  static codeView cv;
  char prefix=0;
  GdkColor white, black,red;
  GdkFont *fontBold;// = gdk_font_load(i8086_GUI_DEFAULT_FONT);//i8086_GUI_DISASM_FONT_BOLD);
  
  i8086_GET_GTKFONT(fontBold, i8086_GUI_DISASM_FONT_BOLD);
  if (fontBold==NULL)
    return; 
  
  i8086_SET_COLOR(white, 65535, 65535, 65535);
  i8086_SET_COLOR(red, 65535, 0, 0);
  i8086_SET_COLOR(black, 0, 0, 0);
    
  rowstr[0] = '\0';
  strHeight=fontBold->ascent+fontBold->descent;
  rowCount=((widget->allocation.height)/strHeight);
  IP = CAST_TO_MEMSIZE(core->pc+i8086GetSegRegister_fast(core, i8086_REG_CS, 1));
   
  if ( (IP > cv.endAdr) || (IP < cv.startAdr) || reset ) /* Viewersegment setzen */
    startAdr = IP;                                       /* pc ist ueber bzw. unter Windowgrenzen gekommen */
  else
    startAdr = cv.startAdr;
   
  gtk_text_freeze(GTK_TEXT(widget));
  i8086guiClearText(widget);
  
  for (i=0; i<rowCount; i++)
  {
    opcode = core->mem[CAST_TO_MEMSIZE(startAdr+read)];  /* opcode lesen */
    
    if (isPrefix(opcode)) /* Prefix-behandlung */
    {
      prefix=1;
      read++;
      cmddata[0]=opcode;
      opcode = core->mem[CAST_TO_MEMSIZE(startAdr+read)];  /* opcode nach prefix lesen */
    }
    else
      prefix=0;
      
    if (cmds[opcode]==NULL) /* illegaler Opcode */
    {
      gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, "ILGOPC", -1);
      break;  
    }
    
    cmddata[0+prefix]=opcode;
    cmdsize=cmds[opcode]->size;
    
    for (j=1; j<cmdsize; j++)            /* Parameter byteweise einlesen */
      cmddata[j+prefix] = core->mem[CAST_TO_MEMSIZE(startAdr+read+j)];

    if (cmds[opcode]->hasMod!=0)
    {
      cmdsize+=getAdditionalCmdLength(core, opcode, cmddata[1+prefix]);

      for (j=cmds[opcode]->size; j<cmdsize; j++) /* Parameter byteweise einlesen */
        cmddata[j+prefix] = core->mem[CAST_TO_MEMSIZE(startAdr+read+j)];
    }
    
    //if (!prefix)
    //{
      
      sprintf(rowstr, "%06x", CAST_TO_MEMSIZE(startAdr+read-prefix)); /* Adressen ausgeben */
      gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, " ", -1);
      if (IP==startAdr+read-prefix) /* aktuellen OpCode sichtbar machen */
       gtk_text_insert(GTK_TEXT(widget), fontBold, &white, &black, rowstr, -1);
      else if(isBreakpoint(CAST_TO_MEMSIZE(startAdr+read-prefix)))/*Breakpoints sichtbar machen*/
       gtk_text_insert(GTK_TEXT(widget), fontBold, &white, &red, rowstr, -1);
      else
        gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, rowstr, -1);
    //}
    //else
      //prefix=0;
    gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, " ", -1);
      
    //gtk_text_insert(widget, fontBold, NULL, NULL, rowstr, -1);
    
    disasm (cmddata, rowstr, 16, startAdr+read, 0, 0);

    gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, rowstr, -1);
    
    //if (isPrefix(opcode))
      //prefix=1;
    //if (!prefix)
    gtk_text_insert(GTK_TEXT(widget), fontBold, NULL, NULL, "\n", -1);
      
    read += cmdsize;
    
    if (i==rowCount-2)
      cv.endAdr = startAdr+read;
  }
  cv.startAdr = startAdr;
  
  gtk_text_thaw(GTK_TEXT(widget));
}

//Signalhandler für  Grössenänderung des Memviewers
void on_memviewer_size_allocate (GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data)
{
  paint_mem(memviewer, NULL, core, gtk_spin_button_get_value_as_int((GtkSpinButton*)memSpinButton));
}

//Signalhandler für  Grössenänderung des Disasmviewer
void on_disasmviewer_size_allocate (GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data)
{
  paint_disasm(disasmviewer, NULL, core, commands, 1);
}

/* Autor: RD */
/* die Memvieweradresseingabe ist über eine Kombination von Spinbutton und Textfield realisiert, */
/* da das Spinbutton Widget keine Hexdarstellung unterstützt */
/* der Spinbutton wird nur für die Buttons mit den Pfeilen benutzt */
void on_memspin_change_value(GtkWidget  *widget, GdkRectangle *allocation, gpointer user_data)
{
  char adr[10];
  sprintf(adr,"%04X:%04X",gtk_spin_button_get_value_as_int((GtkSpinButton*)widget)/0x10000,gtk_spin_button_get_value_as_int((GtkSpinButton*)widget)%0x10000);
  gtk_entry_set_text (GTK_ENTRY (memTextField), adr);
  paint_mem(memviewer, NULL, core, gtk_spin_button_get_value_as_int((GtkSpinButton*)widget));
}

/* Autor: RD */
void on_memtextfield_change_value(GtkWidget  *widget, GdkRectangle *allocation, gpointer userdata)
{
	unsigned short adr;
	unsigned short seg;
	sscanf(gtk_entry_get_text (GTK_ENTRY (memTextField)), "%4hx:%4hx",&seg, &adr);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON (memSpinButton), (seg*0x10000)+adr);
}

//Autor: FB 
//Hilfs-Funktion für changeMemReg (schreibt in variable selectReg den Wert für das Reg)
void changeReg(GtkMenuItem *menuitem,gpointer user_data)
{ 
  unsigned short data;
  unsigned char hexdata[5];
  i8086SingleRegister buf;
  selectReg=(unsigned char)(int) user_data;//ich musste das so casten, weil sonst der gcc ne warning bringt...
  
  if(selectReg>7 && selectReg!=99)//SegRegister
  {
          data=i8086GetSegRegister_fast(core, selectReg-8, 0);
  }
  else if(selectReg==99)//content of IP
  	  data=core->pc;
  else//normales Register
  {
          i8086GetRegisterRef(&buf,core,1,selectReg);
          data=buf.x;
  }
  sprintf(hexdata,"%04X",data);
  gtk_entry_set_text((GtkEntry *)valuetextfield,hexdata);
}

/*Autor:CST*/
/*event wenn eine adresse zum aendern eingegeben wird - es wird der aktuelle wert an der adresse angezeigt*/
void changeMem(GtkEntry *entry,gpointer user_data)
{ 
  unsigned short data,adr,isvalid,seg;
  unsigned char hexdata[5];
  unsigned char *hexadr;
    
  hexadr=(char *)gtk_entry_get_text((GtkEntry *)adrtextfield);
  isvalid=isValidHex(hexadr);
  if(isvalid && strlen(hexadr))
  {
          sscanf(hexadr,"%2hX:%4hX",&seg,&adr);
	  data=core->mem[adr+(seg*0x10000)]; 
  }
  else
	  data=0;
  sprintf(hexdata,"%02X",data);
  gtk_entry_set_text((GtkEntry *)valuetextfield,hexdata);
}

//Autor: FB 
//ändert speicher bzw. Register Inhalte
void changeMemReg (GtkWidget *widget,gpointer rm)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *stock;
  GtkWidget *table;
  GtkWidget *label;
  gint response;
  GtkWidget *menu;
  GtkSizeGroup *size_group;
  GtkWidget *option_menu;
  unsigned short i,v,adr,isvalid=1,seg;
  unsigned char *vhex,*adrhex;
  const char *regnames[] = {"AX", "BX", "CX", "DX", "SI", "DI",
  											"SP", "BP", "CS","DS","ES","SS", "IP",NULL };
  unsigned char regs[] = {i8086_REG_AX,i8086_REG_BX,i8086_REG_CX,i8086_REG_DX,i8086_REG_SI,i8086_REG_DI,i8086_REG_SP,i8086_REG_BP,i8086_REG_CS+8,i8086_REG_DS+8,i8086_REG_ES+8,i8086_REG_SS+8,99};//zur unterscheidung der segmentregister wurde 8 addiert
  
 	//Dialog-Fenster mit Buttons erstellen
  dialog = gtk_dialog_new_with_buttons ("Reg-Mem change",
  			 	 GTK_WINDOW (window),
		 			 GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
	 				 GTK_STOCK_OK, GTK_RESPONSE_OK,
           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
 					 NULL);
 	//Horizontale-Box
  hbox = gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

  //Frage Bild
  stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  
  if(rm==0)//Register ändern
	{  
  	//Register aus wahl
  	menu = gtk_menu_new ();//Menü erstellen
  	for (i=0; i<13; i++)
   	{
      GtkWidget *menu_item = gtk_menu_item_new_with_label (regnames[i]);
      gtk_widget_show (menu_item);
      g_signal_connect(GTK_OBJECT (menu_item), "activate", G_CALLBACK (changeReg),(gpointer)(int) regs[i]);//ich musste das so casten, weil sonst der gcc ne warning bringt...
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
        }

  	option_menu = gtk_option_menu_new ();//neues Options Menü
  	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu); //Menü ins Options Menü einbinden
  	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  
	  label = gtk_label_new_with_mnemonic ("Register");
	  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1,                  0, 1,
		    GTK_EXPAND | GTK_FILL, 0,
		    0,                     0);
  
  	gtk_label_set_mnemonic_widget (GTK_LABEL (label), option_menu);
  	gtk_size_group_add_widget (size_group, option_menu);
  	gtk_table_attach (GTK_TABLE (table), option_menu,
		    1, 2,                  0,  1,
		    GTK_EXPAND | GTK_FILL,                     0,
		    0,                     0);
  }
  else//speicheradr ändern
	{
		label = gtk_label_new_with_mnemonic ("Memory Address (hex)");
  	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
		adrtextfield = gtk_entry_new ();
   	gtk_table_attach (GTK_TABLE (table), adrtextfield, 1, 2, 0, 1,GTK_EXPAND | GTK_FILL,0,0,0);
        gtk_entry_set_max_length        ((GtkEntry *)adrtextfield, 7);
	gtk_entry_set_text ((GtkEntry *)adrtextfield, "00:0000");
        gtk_widget_set_size_request (adrtextfield, 100, 20);
        g_signal_connect(GTK_ENTRY(adrtextfield), "changed", G_CALLBACK (changeMem),NULL);
  }  

   //Inhalt
  label = gtk_label_new_with_mnemonic ("Value (hex)");
  gtk_table_attach_defaults (GTK_TABLE (table), label,  0, 1, 1, 2);
  
  valuetextfield = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE (table), valuetextfield, 1, 2, 1, 2,GTK_EXPAND | GTK_FILL,0,0,0);  
  gtk_entry_set_max_length((GtkEntry *)valuetextfield, 4);
	
  //Fenster inhalt sichtbar machen
  gtk_widget_show_all (hbox);
  gtk_widget_set_size_request (valuetextfield, 100, 20);
  if(!rm)//reg
     changeReg(NULL,0);
  
  //Dialog starten und response mit diaog Verknüpfen
  response = gtk_dialog_run (GTK_DIALOG (dialog));
 
  //wenn OK gedrückt wurde
	if (response == GTK_RESPONSE_OK)
 	{
	 	if(rm==0)//register setzen
	 	{
		 	vhex=(char *)gtk_entry_get_text((GtkEntry*)valuetextfield);
                        isvalid=isValidHex(vhex);
                        if(!isvalid || strlen(vhex)>4 || !strlen(vhex))
                                i8086guiMessage((GtkWidget *) dialog, "Please enter a valid hex-value (4digits 0-f)!","", GTK_MESSAGE_ERROR);
                        sscanf(vhex, "%4hx", &v);
                        
 			//Register setzen
  		if (selectReg>7 && selectReg!=99)//Segmentregister
				i8086SetSegRegister(core, selectReg-8, v);
		else if(selectReg==99) //change IP
				core->pc=v;
 		else//normales register
			        i8086SetRegister(core, selectReg, 1, v);
		}
		else//Speicher setzen
		{
			adrhex=(char *)gtk_entry_get_text((GtkEntry*)adrtextfield);
			vhex=(char *)gtk_entry_get_text((GtkEntry*)valuetextfield);
                        isvalid=isValidHex(vhex);
                        if(!isvalid || strlen(vhex)>2 || !strlen(vhex))
                                i8086guiMessage((GtkWidget *) dialog, "Please enter a valid hex-value (2digits 0-f)!","", GTK_MESSAGE_ERROR);
                        isvalid=isValidHex(adrhex);
                        if(!isvalid || strlen(adrhex)>7 || !strlen(adrhex))
                                i8086guiMessage((GtkWidget *) dialog, "Please enter a valid hex-address (2:4 digits 0-f)!","", GTK_MESSAGE_ERROR);
                        sscanf(vhex, "%2hx", &v);
                        sscanf(adrhex, "%2hx:%4hx", &seg,&adr);                        
			core->mem[adr+(seg*0x10000)] = v;
		}
		
		//neu zeichen der Register und des Speichers
		i8086guiPaintAll();
  }
	
  //Dialog Fenster schliessen
  gtk_widget_destroy (dialog);
}

//Autor: FB
//About Fenster
void about_win(GtkWindow  *widget)
{
	GtkWidget *dialog,*label,*vbox,*wid;
        GdkPixbuf *i8086icon;
        char *str;
        const char *locale = NULL;
        str=(char*)calloc(1, sizeof(char));
        str=stringAdd(str,"A multi-platform i8086 Emulator (SBC-86)\n",NULL);
        str=stringAdd(str,"<b>Version ",VERSION_NUMBER,"</b>\n\n",NULL);
        str=stringAdd(str,get_cpu_str (),"\n",NULL);
        g_get_charset (&locale);
        str=stringAdd(str,"<b>Charset</b>: ",locale," <b>Renderer</b>: ",NULL);
         if(i8086ReadBoolConfig(CONFIG_FILE, "USEXFT", 1)==1)
           str=stringAdd(str,"Xft\n",NULL);
         else
           str=stringAdd(str,"Pango\n",NULL);
	str=stringAdd(str,"<b>compiled</b>: ",__DATE__,"\n\n",NULL);
        str=stringAdd(str,"\302\251 2004 Joerg Mueller-Hipper, Robert Dinse,\n Fred Brodmueller, Christian Steineck\n\nhttp://sourceforge.net/projects/i8086emu/\n",NULL);
        //Dialog Fenster erstellen
          label = gtk_label_new (NULL);
          gtk_label_set_selectable (GTK_LABEL (label), TRUE);
          dialog = /*gtk_dialog_new ();*/
	  gtk_dialog_new_with_buttons ("About i8086emu",
  			 	 GTK_WINDOW (window),
		 			 GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
	 				 GTK_STOCK_OK, GTK_RESPONSE_CANCEL,NULL);

	  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
          vbox = GTK_DIALOG (dialog)->vbox;
          i8086icon=gdk_pixbuf_new_from_file (PIXDIR"/i8086icon.png", 0);
          wid = gtk_image_new_from_pixbuf (i8086icon);
	  gtk_container_add (GTK_CONTAINER (vbox), wid);
          gtk_container_add (GTK_CONTAINER (vbox), label);

          gtk_label_set_markup (GTK_LABEL (label), str);
          gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);

          gtk_widget_show_all (dialog);
          gtk_dialog_run (GTK_DIALOG (dialog));
	  gtk_widget_destroy (dialog);
  	  free(str);
}

/*Autor: CST*/
/*schreibt alle elemente aus breakpoints in die gui liste*/
void updateTree()
{
        char currentbp[8];//string der den hexwert erhaelt
        GtkTreeIter iter;
        int i;
        
        
        gtk_list_store_clear (store);//löscht den store
                         for (i = 0; i < bpelements; i++) //schreibt die elemente aus dem breakpointsarray neu in den store
                        {
                         gtk_list_store_append (store, &iter);                 
                         sprintf(currentbp, "%02x:%04x", breakpoints[i]/0x10000,breakpoints[i]%0x10000);//umwandlung des char in einen string
                         gtk_list_store_set (store, &iter, 0, (i+1), 1, (gchar *)currentbp,  -1);
                        }
 }

/*Autor: CST*/
/*zeichnet das Dialogfenster zum setzen eines Breakpoints*/
void setBreakpoint(GtkWindow  *widget,gpointer callback_data)
{
  GtkWidget *dialog_vbox1;
  GtkWidget *dialog1;
  GtkWidget *table1;
  GtkWidget *label2;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview1;
  GtkWidget *label3;
  GtkWidget *dialog_action_area1;
  GtkWidget *addbreakbutton;
  GtkWidget *button3;
  GtkWidget *button4;
  enum
         {
            NUMBER_COL,
            BP_COL,
            N_COLUMNS
         };
     GtkCellRenderer   *renderer;
     GtkTreeViewColumn *col;
  
  dialog1 = gtk_dialog_new ();//neues dialogfenster erstellen
  gtk_window_set_title (GTK_WINDOW (dialog1), "Breakpoints");//titel setzen
  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;//vbox initialisieren
  gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);//dialog soll modal sein
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);//dialog soll nicht resizeable ein
  gtk_window_set_transient_for(GTK_WINDOW (dialog1),GTK_WINDOW (window));//dialog soll immer im vordergurnd bleiben
  gtk_widget_show (dialog_vbox1);//vbox sichtbar machen

  table1 = gtk_table_new (2, 2, FALSE);//tabelle fuer die dialog elemente
  gtk_widget_show (table1);//tabelle sichtbar machen
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 10);//10 pixel borderbreite
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);//5pixel row-spacing in der tabelle
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);//5pixel row-spacing in der tabelle

  label2 = gtk_label_new ("Breakpoints");//label fuer die breakpoint liste
  gtk_widget_show (label2);//label sichtbar machen
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 0, 1,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);//label in die tabelle einfuegen
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);//ausrichtung links in der tabelle
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);//ausrichtung links in der tabelle
  gtk_misc_set_padding (GTK_MISC (label2), 10, 10);//ausrichtung links in der tabelle

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);//scrolled window fuer die breakpointliste
  gtk_widget_show (scrolledwindow1);//sichtbar machen
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow1, 1, 2, 0, 1,(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),(GtkAttachOptions)(GTK_FILL), 0, 0);//window in die tabelle einfuegen
  gtk_widget_set_size_request (scrolledwindow1, 180, 150);//groesse von 150x150 fuer das scrolledwindow anfordern
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);//horizontale scrollbars nach bedarf anzeigen,horizontale immer
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);//schattentyp fuer das window

  /*Hier wird die Liste der Breakpoints auf basis eines TreeViews mit ListStore als Model erzeugt*/
         store = gtk_list_store_new (N_COLUMNS, G_TYPE_INT, G_TYPE_STRING);//neuer liststore
         updateTree();//inhalt des breakpoint arrays in den liststore schreiben
         treeview1 = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));//treeview mit dem liststore model initialisieren
         g_object_unref (G_OBJECT (store));//liststore free
        renderer = gtk_cell_renderer_text_new ();//renderer initialisieren
         col = gtk_tree_view_column_new_with_attributes ("Number", renderer, "text", NUMBER_COL, NULL);//spalte "nummer"
         gtk_tree_view_append_column (GTK_TREE_VIEW (treeview1), col);//in den tree einfuegen
         col = gtk_tree_view_column_new_with_attributes  ("Address", renderer, "text", BP_COL, NULL);//spalte "adresse"
         gtk_tree_view_append_column (GTK_TREE_VIEW (treeview1), col);//in den tree einfuegen
         g_signal_connect ((gpointer) treeview1, "cursor-changed",G_CALLBACK (i8086event_rowchanged),(GtkWidget *)treeview1);//eventfunktion registrieren (fuer mausklick auf listenelement)
 /*ende des treeviews*/
  gtk_widget_show (treeview1);//treeview sichtbar machen
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);//treeview in das window einfuegen
  label3 = gtk_label_new ("New breakpoint");//label fuer das eingabefeld
  gtk_widget_show (label3);//sichtbar machen
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 1, 2,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);//label in die tabelle einfuegen
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);//ausrichtung des labels
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);//ausrichtung des labels
  gtk_misc_set_padding (GTK_MISC (label3), 10, 10);//ausrichtung des labels

  bpadrtextbox = gtk_entry_new ();//neues textfeld
  gtk_entry_set_text((GtkEntry *)bpadrtextbox,"00:0000");
  gtk_widget_show (bpadrtextbox);//sichtbar machen
  gtk_table_attach (GTK_TABLE (table1), bpadrtextbox, 1, 2, 1, 2,(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),(GtkAttachOptions) (0), 0, 0);//textfeld in die tabelle einfuegen

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;//feld fuer die buttons
  gtk_widget_show (dialog_action_area1);//sichtbar machen
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);//layout auf buttonbox

  addbreakbutton = gtk_button_new_from_stock ("gtk-add");//add breakpoint button
  gtk_widget_show (addbreakbutton);//sichtbar machen
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), addbreakbutton, 0);//button zum buttonfeld hinzufuegen
  GTK_WIDGET_SET_FLAGS (addbreakbutton, GTK_CAN_DEFAULT);

  button3 = gtk_button_new_from_stock ("gtk-remove");//del breakpoint button
  gtk_widget_show (button3);//sichtbar machen
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button3, 0);//button zum buttonfeld hinzufuegen
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  button4 = gtk_button_new_from_stock ("gtk-close");//close button
  gtk_widget_show (button4);//sichtbar machen
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button4, GTK_RESPONSE_CLOSE);//button zum buttonfeld hinzufuegen
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);
  
  g_signal_connect ((gpointer) addbreakbutton, "clicked",G_CALLBACK (i8086event_addbp),(GtkWidget *)dialog1);//signalhandler fuer add-button registrieren
  g_signal_connect ((gpointer) bpadrtextbox, "activate",G_CALLBACK (i8086event_addbp),(GtkWidget *)dialog1);//signalhandler fuer textbox registrieren
  g_signal_connect ((gpointer) button3, "clicked",G_CALLBACK (i8086event_delbp),NULL);//signalhandler fuer del-button registrieren
  g_signal_connect ((gpointer) button4, "clicked", G_CALLBACK (i8086event_closeDialog),dialog1);//signalhandler fuer close-button registrieren
gtk_widget_show_all (dialog1);//dialog sichtbar machen
}
