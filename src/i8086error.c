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

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include "i8086messages.h"
#include "i8086error.h"
#include "i8086util.h"

/* Gibt msg1 und msg2 durch \n getrennt auf dem */
/* Bildschirm aus. Beendet mit endwin() noch    */
/* offene curses-windows.                       */
void i8086errorEx(const char *filename, int linenum, const char *msg1, const char *msg2)
{
  FILE *file;
  char lnum[16];
  char *linfilename;
  
  #ifdef _WIN32
    file = fopen(i8086_LOG_FILE, "a");
  #else
  linfilename=(char*)calloc(1, sizeof(char));
  linfilename=stringAdd(linfilename,getenv("HOME"),"/.i8086emu/",i8086_LOG_FILE,NULL);
  file = fopen(linfilename, "a");
  #endif
  if (file!=NULL)
  {
    fputs("[Error] ", file);
    fputs(filename, file);
    fputs(": Line ", file);
    sprintf(lnum, "%d", linenum);
    fputs(lnum, file);
    fputs(": ", file);
    fputs(msg1, file);
    fputs(": ", file);
    fputs(msg2, file);
    fputc('\n', file);
    fclose(file);
  }
  
  printf("\n\n%s\n%s\n", msg1, msg2);
  raise(SIGQUIT);
}

/* Schreibt eine Warnung in das Log-File        */
/* filename: Name der Datei in der die Warnung  */
/*           aufgetreten ist.                   */
/* linenum : Zeilennummer in der die Warnung    */
/*           aufgetreten ist.                   */
/* msg     : Nachricht der Warnung.             */
void i8086warningEx(const char *filename, int linenum, const char *msg)
{
  FILE *file;
  char lnum[16];
  char *linfilename;
  
  #ifdef _WIN32
    file = fopen(i8086_LOG_FILE, "a");
  #else
  linfilename=(char*)calloc(1, sizeof(char));
  linfilename=stringAdd(linfilename,getenv("HOME"),"/.i8086emu/",i8086_LOG_FILE,NULL);
  file = fopen(linfilename, "a");
  #endif
  if (file!=NULL)
  {
    fputs("[Warning] ", file);
    fputs(filename, file);
    fputs(": Line ", file);
    sprintf(lnum, "%d", linenum);
    fputs(lnum, file);
    fputs(": ", file);
    fputs(msg, file);
    fputc('\n', file);
    fclose(file);
  }
}

/* Leert das Log-File */
void i8086clearLog()
{
  FILE *file;
  char *linfilename;
  #ifdef _WIN32
  file = fopen(i8086_LOG_FILE, "w+");
  #else
  linfilename=(char*)calloc(1, sizeof(char));
  linfilename=stringAdd(linfilename,getenv("HOME"),"/.i8086emu/",i8086_LOG_FILE,NULL);
  file = fopen(linfilename, "w+");
  #endif
  fclose(file);   
}
