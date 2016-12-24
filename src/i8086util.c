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
/* Version: 1.0                                     */
/*                                                  */
/* kleine Hilfsfunktionen			                      */
/*                                                  */
/****************************************************/

#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i8086proc.h"
#include "i8086util.h"
#include "i8086error.h"


/* Autor: RD */
/* Liefert length bits ab bit pos aus b. */
/* Bsp: getBitSnipped(b, 5, 3) liefert   */
/*      die bits die mit x markiert sind */
/*      Pos: 76543210                    */
/*             |->                       */
/*           00xxx000                    */
/*
unsigned char getBitSnipped(unsigned char b,unsigned char pos, unsigned char length)
{
	unsigned char result;
	result=b << (7-pos);
	result=result >> (7-(length-1));
	return result; 
}
*/

/* Verkettet s1 und s2 indem s1 um die Laenge von s2 */
/* erweitert wird. Gibt neues s1 zurueck.						 */
char *stringAdd(char *s1, ...)
{
	char *substr;
	va_list vlist; /* variable Argumentliste vorbereiten */
	
	va_start(vlist, s1); /* variable Liste starten */

	for(;;) 
	{
		substr=va_arg(vlist, char *); /* nächster Parameter */
		if(substr == NULL) break; /* NULL -> fertig! */
		//len=len+strlen(substr); /* neue Länge */
		//result = (char *)realloc(result, len); /* reservieren */
		//strcat(result, substr); /* Parameter anhängen */
		s1 = (char *)realloc(s1, strlen(s1)+strlen(substr)+1); /* reservieren */
		strcat(s1, substr);
	}
	va_end(vlist);
	
	return s1;
}	



/* Autor: RD */
/* vertauscht die oberen 8bit mit den unteren 8bit von i*/
/*
unsigned short swapbytes(unsigned short i) //vertauscht die unteren 8 Bit mit den oberen 8 Bit eines Words
{
	return (i<<8) | (i>>8);
}
*/
/*unsigned short joinBytes(unsigned char a,unsigned char b) //f?gt 2 bytes zu einem word zusammen, a high, b low
{
	unsigned short res;
	res=a;
	res=res<<8;
	res=res|b;
}
*/

/* JMH                      */
/* sleep fuer Mikrosekunden */
void delay(long microsec)
{
  struct timeval timeout;
  
  timeout.tv_sec = microsec / 1000000L;
  timeout.tv_usec = microsec % 1000000L;
  select(0, NULL, NULL, NULL, &timeout);
}

/* JMH                                                                        */
/* Berechnet die Zeitdiff. zw 8086- und emu-Ausfuehrungsgeschwindigkeit       */
/* und setzt den emu gegebenenfalls in den sleep-modus bis die                */
/* Ausfuehrungsgeschwindigkeit wieder synchron sind.                          */
/* clocks   Takte die der 8086 benoetigen wuerde.                             */
/* usedTime Zeit(micSec) die der emu fuer die Funktionen der Takte benoetigte */
void cmdWait(unsigned int clocks, unsigned long usedTime)
{
  signed long procTime, waitTime;

  procTime = clocks*i8068_TPI;  /* Zeit die 8086 benoetigen wuerde*/
  waitTime = procTime-usedTime; /* Zeitdiff. zw emu und 8086 -> muss gewartet werden. */
  if (waitTime > 0)             /* emu war schneller als 8086 -> muss warten */
  {
    //usleep(waitTime);
    delay(waitTime);
  }
 }
  
/*copys the file src to file dest and creates file dest if it is not allready there*/
/*returns 1 if copy failed*/
/*returns 0 if copy was a success*/
int copyFile(char *src, char *dest)
{
 #define BUFFSIZE 1024
 int readblocks;
 FILE *infile,*outfile;
 void *buffer;
 
 	infile=fopen(src,"r"); //quelldatei oeffnen
	if(infile==NULL)
    		return 1;
   	outfile=fopen(dest,"w"); //Zieldatei erstellen
    	buffer=malloc(BUFFSIZE*sizeof(char));

	if(outfile==NULL)
	   return 1;
while(!feof(infile))
{
    readblocks=fread(buffer,sizeof(char),BUFFSIZE,infile);
    fwrite(buffer,sizeof(char),readblocks,outfile);
}

free(buffer);
fclose(infile);
fclose(outfile);
return 0;
} 

