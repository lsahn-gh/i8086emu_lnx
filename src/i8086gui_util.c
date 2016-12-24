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

#include <gtk/gtk.h>
#include <sys/utsname.h>
#include <glib.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "i8086gui_util.h"       

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


int i8086getFontHeight(GtkWidget *widget)
{
  PangoContext *context;
  PangoFontMetrics *metrics;

  context = gtk_widget_get_pango_context(widget);
  metrics = pango_context_get_metrics(context, widget->style->font_desc,
                                      pango_context_get_language(context));
  return PANGO_PIXELS(pango_font_metrics_get_ascent(metrics));
}

int i8086getTextWidth(GtkWidget *widget, const char *text)
{
  PangoContext *context;
  PangoFontMetrics *metrics;

  context = gtk_widget_get_pango_context(widget);
  metrics = pango_context_get_metrics(context, widget->style->font_desc,
                                      pango_context_get_language(context));
  return PANGO_PIXELS(pango_font_metrics_get_approximate_char_width(metrics))*strlen(text); 
}

void i8086guiClear(GtkWidget *widget)
{
  GdkGC *gc3; // "Farbspeicher"
  GdkColor color;
  
  gc3 = gdk_gc_new (widget->window);
  color.red   = 65535;
  color.green = 65535;
  color.blue  = 65535;
  gdk_gc_set_rgb_fg_color (gc3, &color);
  
  gdk_draw_rectangle(widget->window, gc3, TRUE, 
                     0, 0, widget->allocation.width, widget->allocation.height);
}

/* Loescht den Inhalt eines GtkText-Fensters  */
void i8086guiClearText(GtkWidget *text)
{
  struct GtkText *itext=(struct GtkText *)text;
  gtk_text_set_point(GTK_TEXT(itext) ,0);
  gtk_text_forward_delete(GTK_TEXT(itext), gtk_text_get_length(GTK_TEXT(itext)));
}

/*Autor: CST*/
/*fuegt einen Breakpoint in das BP-Array hinzu*/
/*gibt 1 zurueck fuer erfolgreiches einfuegen*/
/*gibt 0 zurueck falls das neue element bereits enthalten ist*/
unsigned char addBp(unsigned int bpoint)
{
         if(isBreakpoint(bpoint))//ist der bp schon vorhanden wird abgebrochen
         {
                 return 0;//element war schon vorhanden
         }
         else
         {
	        breakpoints=(unsigned int *)realloc(breakpoints,(1+bpelements)*sizeof(unsigned int));//array um ein bp vergroessern
                breakpoints[bpelements]=bpoint;//element hinten anfuegen
                bpelements++;
                return 1;//element eingefuegt
          }
}

/*Autor: CST*/
/*entfernt einen Breakpoint aus dem BP-Array*/
/*gibt 0 zurueck falls das element nicht im array enthalten ist*/
/*gibt 1 zurueck fuer erfolgreiches loeschen*/
unsigned char delBp(unsigned int bpoint)
{
        unsigned int i;
               
        for(i=0;i<bpelements;i++)
        {
                if(breakpoints[i]==bpoint)//wenn das element gefunden wurde
                        {
                                memmove(&breakpoints[i], &breakpoints[i+1], (bpelements-i-1)*sizeof(unsigned int));//element entfernen und rest nach vorn schieben
                                breakpoints=(unsigned int *)realloc((void *)breakpoints,(bpelements-1)*sizeof(unsigned int));//das array um einen bp verkleinern
                                bpelements--;
                                return 1; //element wurde geloescht
                        }
         }
        return 0; //element nicht vorhanden
}

/*Autor: CST*/
/*prueft ob eine adresse ein breakpoint ist*/
/*gibt 1 zurueck falls es ein breakpoint ist*/
/*gibt 0 zurueck falls es kein breakpoint ist*/
unsigned char isBreakpoint(unsigned int adr)
{
unsigned int i,isbp=0;

for(i=0;i<bpelements;i++)//sucht im breakpoint array nach der uebergebenen adresse
{
             if(adr==breakpoints[i])
             {
                     isbp=1;//breakpoint gefunden - rueckgabe 1 
                     break;
              }
}
return isbp;
}

/*Autor:CST*/
/*ueberprüft ob ein string eine hexzahl ist*/
/*liefert 0 falls nicht, sonst 1*/
unsigned char isValidHex(char *str)
{
        unsigned char i,ret=1;
                        for(i=0;i<strlen(str);i++)
                        {
                                if(!isxdigit(str[i]) && str[i]!=':')//ist der aktuelle buchstabe keiner von 0-f oder:
                                {
                                        ret=0;// rueckgabewert 0
                                        break;//abbrechen
                                }
                        }
return ret;
}



/*alle folgenden Funktionen entstammen unverändert dem Sourcecode für X-Chat 2.0.6 (c) 1998-2003 Peter Zelezny und wurden nur */
/*fuer die About-box benutzt*/
/*http://www.xchat.org*/
/*Diese Zeilen stehen unter der GPL*/
#ifdef _WIN32
int get_mhz (void)
{
	HKEY hKey;
	int result, data, dataSize;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\"
		"CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		dataSize = sizeof (data);
		result = RegQueryValueEx (hKey, "~MHz", 0, 0, (LPBYTE)&data, &dataSize);
		RegCloseKey (hKey);
		if (result == ERROR_SUCCESS)
			return data;
	}
	return 0;	/* fails on Win9x */
}

char * get_cpu_str (void)
{
	static char verbuf[64];
	OSVERSIONINFO osvi;
	SYSTEM_INFO si;
	int mhz;

	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	GetVersionEx (&osvi);
	GetSystemInfo (&si);

	mhz = get_mhz ();
	if (mhz)
		sprintf (verbuf, "Windows %ld.%ld [i%d86/%dMHz]",
			osvi.dwMajorVersion, osvi.dwMinorVersion, si.wProcessorLevel, mhz);
	else
		sprintf (verbuf, "Windows %ld.%ld [i%d86]",
			osvi.dwMajorVersion, osvi.dwMinorVersion, si.wProcessorLevel);

	return verbuf;
}

#else
char * get_cpu_str (void)
{
	int mhz, cpus;
	struct utsname un;
	static char *buf = NULL;

	if (buf)
		return buf;

	buf = malloc (128);

	uname (&un);

	get_cpu_info (&mhz, &cpus);
	if (mhz)
		snprintf (buf, 128,
					 (cpus == 1) ? "%s %s [%s/%dMHz]" : "%s %s [%s/%dMHz/SMP]",
					 un.sysname, un.release, un.machine, mhz);
	else
		snprintf (buf, 128, "%s %s [%s]", un.sysname, un.release, un.machine);

	return buf;
}
#endif

void get_cpu_info (int *mhz, int *cpus)
{

	char buf[256];
	int fh;

	*mhz = 0;
	*cpus = 0;

	fh = open ("/proc/cpuinfo", O_RDONLY);	/* linux 2.2+ only */
	if (fh == -1)
	{
		*cpus = 1;
		return;
	}

	while (1)
	{
		if (waitline (fh, buf, sizeof buf) < 0)
			break;
		if (!strncmp (buf, "cycle frequency [Hz]\t:", 22))	/* alpha */
		{
			*mhz = atoi (buf + 23) / 1048576;
		} else if (!strncmp (buf, "cpu MHz\t\t:", 10))	/* i386 */
		{
			*mhz = atof (buf + 11) + 0.5;
		} else if (!strncmp (buf, "clock\t\t:", 8))	/* PPC */
		{
			*mhz = atoi (buf + 9);
		} else if (!strncmp (buf, "processor\t", 10))
		{
			(*cpus)++;
		}
	}
	close (fh);
	if (!*cpus)
		*cpus = 1;
}


int waitline (int sok, char *buf, int bufsize)
{
	int i = 0;

	while (1)
	{
		if (read (sok, &buf[i], 1) < 1)
			return -1;
		if (buf[i] == '\n' || bufsize == i + 1)
		{
			buf[i] = 0;
			return i;
		}
		i++;
	}
}

