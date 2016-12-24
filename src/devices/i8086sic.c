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
/*                                                  */
/* SIC                                              */
/*                                                  */
/****************************************************/

/*
this is for demonstration purposes only
all it does is loading a file into the memory via an fake SIC using the SBC-86 ROM
*/

#include "../i8086devices.h"
#include "../i8086error.h"
#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtktext.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <unistd.h>



i8086msgFunc *oldPortOutHandler;
i8086DeviceCalls *funcCalls;
unsigned int p=0;
unsigned char * data;

void picPortOutHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{

	if (hParam == 0xd0)
	{
		//printf("%02x ",lParam);
		funcCalls->SendMsg(i8086_SIG_PORT_WRITE_VALUE, 0xd0, data[p]);
		p++;
		usleep(600);
	}
	else if (hParam == 0xd2 && lParam == 0x40)
		//sic reset
		p=0;

	if (oldPortOutHandler)
    		oldPortOutHandler(msg, hParam, lParam);
}

char * sic_win()
{
  char *f;
  GtkWidget *file_selection_box = gtk_file_selection_new("Please select a file.");
  gtk_dialog_run (GTK_DIALOG(file_selection_box));
  f=(char *)gtk_file_selection_get_filename ((gpointer)file_selection_box);
  gtk_widget_destroy (file_selection_box);
  return f;
}


void DeviceInit(i8086DeviceCalls *calls)
{
  FILE *file;
  unsigned int fsize=0;
  funcCalls = calls;
  oldPortOutHandler = funcCalls->SetMsgHandler(i8086_SIG_PORT_OUT, picPortOutHandler);
  funcCalls->SendMsg(i8086_SIG_PORT_WRITE_VALUE, 0xd2, 2);
  //funcCalls->SendMsg(i8086_SIG_PORT_WRITE_VALUE, 0xd0, 0xaa);

  data=calloc(15000,1);
  data[0]=0xaa;

  
  file = fopen(sic_win(), "rb");
  fsize = fread(data+2, sizeof(char), 10000, file);
  data[1]=fsize/128;
  if (fsize%128)
  	data[1]++;
  data[(data[1]*128)+3]='G';

  fclose(file);
}
