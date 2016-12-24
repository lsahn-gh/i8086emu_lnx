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
/* Funktionen zum Lesen und Schreiben von           */
/* Config-Files.                                    */
/*                                                  */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "i8086config.h"
#include "i8086util.h"
#include "i8086error.h"


/*opens the config file end returns the FILE pointer to it*/
FILE *i8086OpenConfig(const char *filename)
{
 char *linfilename;
 FILE *file;
  #ifdef _WIN32
  	file = fopen(filename, "r");//if win32 - open the filename in current dir
  #else
  linfilename=(char*)calloc(1, sizeof(char));
  linfilename=stringAdd(linfilename,getenv("HOME"),"/.i8086emu/",NULL);//get homedir incl. filename string
  mkdir(linfilename,S_IRWXU);
  linfilename=stringAdd(linfilename,filename,NULL);//get homedir incl. filename string
  file = fopen(linfilename, "r");//try to open config file
  if(file==NULL)//didn't work
  {
	   copyFile(DATADIR"/"CONFIG_FILE".example",linfilename);//copy standard cfg from datadir to homedir
           file = fopen(linfilename, "r");//open it
  }
  free(linfilename);//all your memory is freed
  #endif
  return file;
}

/* Prueft ob die angegebene Config-Datei existiert.  */
/* Bei Erfolg wird i8086_SUC_FILELOAD zurueckgegeben */
/* sonst i8086_ERR_FILENF.                           */
int i8086ConfigExists(const char *filename)
{
  FILE *file;
  
  file=i8086OpenConfig(filename);
  if (file)
  {
    fclose(file);
    return i8086_SUC_FILELOAD;
  }
  else
    return i8086_ERR_FILENF;
}


/* Liest einen String aus dem Config-File.           */
/* buf: Adresse der Zeichenkette in die gelesen wird */
/* filename: Dateiname der Konfigurationsdatei       */
/* name: Name des Konfigurationseintrages            */
/* Returns: Gibt 1 zurueck wenn der Konfigurations-  */
/*          eintrag gelesen werden konnte, sonst 0.  */
int i8086ReadStrConfig(char *buf, const char *filename, const char *name)
{
  FILE *file;
  char str[i8086_CFG_MAX_VALUE_LEN];
  char *value;
  int ret=0;
  file=i8086OpenConfig(filename);
  buf[0]='\0';
  if (file!=NULL)
  {
    while ( fgets(str, i8086_CFG_MAX_VALUE_LEN, file) != NULL )
    {
      if (str[0]!='#' && str[0]!='\0') /* # - Zeichen fuer Kommentar */
      {
        value = strchr(str, '=');
        if (value!=NULL && strlen(value)>1)
        {
          if (value[strlen(value)-1]=='\n') /* \n entfernen */
            value[strlen(value)-1] = '\0';
          if (value[strlen(value)-1]=='\r') /* \r entfernen */
            value[strlen(value)-1] = '\0';
            
          value = strchr(value, value[1]);
          if ((strlen(value)>0) && (strncmp(str, name, strlen(str)-strlen(value)-1)==0))
          {
            strcpy(buf, value);
            ret=1;
            break;
          }
        }
      } 
    }
    fclose(file);
  }
  return ret;
}

/* Liest einen Hex-Wert aus dem Config-File      */
/* filename: Dateiname der Konfigurationsdatei   */
/* name: Name des Konfigurationseintrages        */
/* defValue: Wert der zurueckgegeben werden soll */
/*           wenn kein korrekter Wert existiert  */
/* Returns: Gibt bei Erfolg den gelesenen Wert   */
/*          zurueck, sonst defValue              */
int i8086ReadHexConfig(const char *filename, const char *name, int defValue)
{
  char str[i8086_CFG_MAX_VALUE_LEN];
  
  if (i8086ReadStrConfig(str, filename, name)!=0)
    return strtoul(str, NULL, 16);
  else
    return defValue;
}

/* Liest einen Dezimal-Wert aus dem Config-File  */
/* filename: Dateiname der Konfigurationsdatei   */
/* name: Name des Konfigurationseintrages        */
/* defValue: Wert der zurueckgegeben werden soll */
/*           wenn kein korrekter Wert existiert  */
/* Returns: Gibt bei Erfolg den gelesenen Wert   */
/*          zurueck, sonst defValue              */
int i8086ReadDecConfig(const char *filename, const char *name, int defValue)
{
  char str[i8086_CFG_MAX_VALUE_LEN];
  
  if (i8086ReadStrConfig(str, filename, name)!=0)
    return strtoul(str, NULL, 10);
  else
    return defValue;
}

/* Liest einen Wahrheits-Wert aus dem Config-File */
/* filename: Dateiname der Konfigurationsdatei    */
/* name: Name des Konfigurationseintrages         */
/* defValue: Wert der zurueckgegeben werden soll  */
/*           wenn kein korrekter Wert existiert   */
/* Returns: Bei 0, N oder NO wird 0 zurueckgegeben*/
/*          , sonst 1                             */
int i8086ReadBoolConfig(const char *filename, const char *name, int defValue)
{
  char *str;
  int len, ret=defValue;

  str = (char*)calloc(i8086_CFG_MAX_VALUE_LEN, sizeof(char));
  
  if (i8086ReadStrConfig(str, filename, name)!=0)
  {
    len=strlen(str);
    strup(str);
    
    if (len==1)
    {
      if ((str[0]=='0') || (str[0]=='N'))
        ret=0;
      else
        ret=1; 
    }
    else
    {
      if (strcmp(str, "NO")==0)
        ret=0;
      else
        ret=1;
    }
  }
  
  free(str);
  
  return ret;
}

/* Prueft ob ein Konfigurationseintrag existiert */
int i8086NameExistsConfig(const char *filename, const char *name)
{
  char str[i8086_CFG_MAX_VALUE_LEN];
  
  return i8086ReadStrConfig(str, filename, name);
}

/* ** */
void i8086WriteStrConfig(const char *filename, const char *name, const char *value)
{
  FILE *file;
  
  file = fopen(filename, "r+");
  if (file!=NULL)
  {
    //fputs("[Error] ", file);
    fclose(file);
  }
}
