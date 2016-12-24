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

#ifndef _i8086ERROR_H_
#define _i8086ERROR_H_

/* Aktiviert die max. Warnungsanzahl */
#define MAX_DEBUG

/* Return Codes */
/*  Datei-Codes */
#define i8086_SUC_FILELOAD 0   /* Datei erfollgreich geladen         */
#define i8086_ERR_FILENF -1    /* Datei konnte nicht geoffnet werden */
#define i8086_ERR_FILEREAD -2  /* Datei konnte nicht gelesen werden  */
/*  Kommando-Ausfuehrungs-Codes */
#define i8086_SUC_EXECCMD 0    /* Kommando erfollgreich ausgefuehrt  */
#define i8086_ERR_ILGOPCODE -1 /* unzulaessiger Opcode               */
#define i8086_ERR_ILGPORT -2   /* unzulaessiger I/O Port             */
/*  Device-Codes */
#define i8086_SUC_DEV_LOAD 1   /* Device wurde erfollgreich geladen  */
#define i8086_ERR_DEV_LOAD 0   /* Fehler beim laden des Devices      */

#define i8086_ERR_OPCODE_ALR_EXIST /* Kommando-Struktur fuer diesen Opcode existiert bereits */

/* String Konstanten */
#define i8086_ERR_STR_FILENF "Datei konnte nicht geoeffnet werden."
#define i8086_ERR_STR_CONFIGNF "Config-Datei konnte nicht geoeffnet werden."
#define i8086_STR_FILELOAD "Datei wurde erfolgreich geladen."
#define i8086_ERR_STR_ILGOPCODE "Opcode not implemented."
#define i8086_ERR_STR_SIGSEGV "Segmentation fault."
#define i8086_ERR_STR_SIG_SET "Signalhandler konnte nicht gesetzt werden"
#define i8086_ERR_STR_CREATE_WIN "Fenster konnten nicht korrekt erstellt werden."
#define i8086_STR_SIZE_TERM "Vielleicht ist Ihre Konsole zu klein. Min.: 23x81"
#define i8086_STR_OPCODE_ALR_EXIST "Kommando-Struktur fuer diesen Opcode existiert bereits."
#define i8086_ERR_OPEN_SEM "Can not open Semaphore."
#define i8086_ERR_PV_SEM "Semaphore-PV-Error."
#define i8086_ERR_INIT_SEM "Cant init Semaphore."
#define i8086_STR_LOAD_DEVICES "Load Devices." 
#define i8086_STR_CLOSE_DEVICES "Close Devices."
#define i8086_ERR_LOAD_FONT "Font konnte nicht geladen werden."
#define i8086_ERR_LOAD_PRIM_FONT "Primaerer Default-Font konnte nicht geladen werden."
#define i8086_ERR_LOAD_SEC_FONT "Sekundaerer Default-Font konnte nicht geladen werden."

#define i8086_LOG_FILE "emu.log"

/* Makros fuer Fkts */
#define i8086warning(msg) i8086warningEx(__FILE__, __LINE__, msg)
#define i8086error(msg1, msg2) i8086errorEx(__FILE__, __LINE__, msg1, msg2)

/* Gibt msg1 und msg2 aus. Verlaesst das Programm mit exit-code 1 */
void i8086errorEx(const char *filename, int linenum, const char *msg1, const char *msg2);
/* Schreibt eine Warnung in das Log-File */
void i8086warningEx(const char *filename, int linenum, const char *msg);
/* Leert das Log-File */
void i8086clearLog();


#endif /* _i8086ERROR_H_ */
