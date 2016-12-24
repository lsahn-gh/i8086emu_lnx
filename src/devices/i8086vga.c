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
 

#include "../i8086devices.h"
#include "../i8086error.h"
#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtktext.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <unistd.h>
#include <time.h>


#define vga_segment 0xa000
#define screen_width 320
#define screen_height 200

typedef struct vgaPixel
{
  unsigned char r, g, b;
}vgaPixel;

i8086msgFunc *oldPortOutHandler;
i8086DeviceCalls *funcCalls;
unsigned int p=0;
vgaPixel palette[256];
GtkWidget *screen;
guchar rgbbuf[screen_width * screen_height * 3];
static guint timeout_id;
//pthread_t vgathread;
//GThread *vgathread;
//static GMutex *drawMutex = NULL;
  
void setColor(unsigned char i,unsigned char red, unsigned char green, unsigned char blue);


void vgaPortOutHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
	static char stage=0;
	static unsigned char colorindex,red,green,blue;

	if (hParam == 0x03c8 || hParam == 0x03c9)
	{
		if (hParam == 0x03c8)
		{
			colorindex=lParam;
			stage=1;
		}
		else if (stage==1)
		{
			red=lParam;
			stage++;
		}
		else if (stage==2)
		{
			green=lParam;
			stage++;
		}
		else if (stage==3)
		{
			blue=lParam;
			stage=0;
			setColor(colorindex,red,green,blue);
		}
	}

	if (oldPortOutHandler)
  		oldPortOutHandler(msg, hParam, lParam);
}

gboolean paint_screen(gpointer widget)
{
  guchar *pos;
  gint x, y;
  unsigned char pixel;
  
//  g_mutex_lock(drawMutex);
  
  pos = rgbbuf;
  for (y = 0; y < screen_height; y++)
  {
    for (x = 0; x < screen_width; x++)
	  {
  	  pixel=funcCalls->MemReadByte(y*screen_width+x, vga_segment);
			*pos++ = palette[pixel].r;
			*pos++ = palette[pixel].g;
			*pos++ = palette[pixel].b;
  	}
  }
    
  //expose_event(screen, NULL);
  //g_signal_emit_by_name(GTK_OBJECT(screen), "expose_event");
  
  gdk_draw_rgb_image(GTK_WIDGET(widget)->window, GTK_WIDGET(widget)->style->fg_gc[GTK_STATE_NORMAL],
		                 0, 0, screen_width, screen_height,
		                 GDK_RGB_DITHER_MAX, rgbbuf, screen_width * 3);

//  g_mutex_unlock(drawMutex);
  
	return TRUE;
}
/*
static gboolean expose_event( GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  g_mutex_lock(drawMutex);

  gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
	                 0, 0, screen_width, screen_height,
	                 GDK_RGB_DITHER_MAX, rgbbuf, screen_width * 3);
                 
	g_mutex_unlock(drawMutex);
	
  return TRUE;
}
*/
void create_vga_win()
{
  GtkWidget *window;
  
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), "VGA 320x200 (256 Colors)");

	screen = gtk_drawing_area_new ();
	gtk_widget_show (screen);
	gtk_container_add (GTK_CONTAINER (window), screen);
	gtk_widget_set_size_request (screen, screen_width, screen_height);

	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);

	//g_signal_connect(GTK_OBJECT(screen), "expose_event", G_CALLBACK(expose_event), NULL);
//i8086warning(g_signal_name(GDK_EXPOSE));

	gtk_widget_show(window);
}

void setColor(unsigned char i,unsigned char red, unsigned char green, unsigned char blue)
{
	palette[i].r = red*4;
	palette[i].g = green*4;
	palette[i].b = blue*4;
}

void initPalette()
{
	int i;

	for (i=0;i<=255;i++)
		setColor(i,i,i,i);
}
/*
gpointer vgaThreadFunction(gpointer data)
{
	while (1)
	{
		usleep(33333);
		paint_screen(screen);
	}
	
	return NULL;
}
*/
void DeviceInit(i8086DeviceCalls *calls)
{
  funcCalls = calls;
  oldPortOutHandler = funcCalls->SetMsgHandler(i8086_SIG_PORT_OUT, vgaPortOutHandler);

  
  create_vga_win();
  initPalette();

  //g_thread_init(NULL);
  //drawMutex = g_mutex_new();
  //vgathread = g_thread_create(vgaThreadFunction, (gpointer)screen, TRUE, NULL);
  //pthread_create(&vgathread, NULL, vgaThreadFunction, (void*)"");
  //gtk_timeout_add (500, (GSourceFunc) paint_screen, NULL);
  timeout_id = g_timeout_add(50, paint_screen, screen);
}

void DeviceClose()
{
  g_source_remove(timeout_id);
}
