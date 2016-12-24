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
/* ** => nicht beendet                              */
/*                                                  */
/* Funktionen fuer die Geraeteschnittstelle         */
/*                                                  */
/****************************************************/

#ifndef _i8086DEVICES_H_
#define _i8086DEVICES_H_

#include "i8086messages.h"
#include "i8086proctypes.h"


/* Handles fuer alle geladenen Devices */
/* Das letzte Element muﬂ NULL sein    */
void **DeviceHandles;
i8086core *currentCore;

typedef unsigned char (i8086MemReadByte)(i808616BitAdr adr, i808616BitAdr seg);
typedef unsigned short (i8086MemReadWord)(i808616BitAdr adr, i808616BitAdr seg);

typedef struct i8086DeviceCalls
{
  char* Version;
  i8086PSetMsgFunc *SetMsgHandler;
  i8086msgFunc *SendMsg;
  i8086MemReadByte *MemReadByte;
  i8086MemReadWord *MemReadWord;
}i8086DeviceCalls;

/* muss in der Dll DeviceInit genannt werden      */
/* diese Funktionsdeklaration muss vorhanden sein */
/* wird beim laden des Device aufgerufen          */
typedef void (i8086DeviceInit)(i8086DeviceCalls *calls);

/* muss in der Dll DeviceClose genannt werden */
/* wird beim schliessen des Device aufgerufen */
/* diese Funktionsdeklaration muss nicht vorhanden sein */
typedef void (i8086DeviceClose)();

/* muss in der Dll DeviceReset genannt werden */
/* wird beim Reseten des Device aufgerufen    */
/* diese Funktionsdeklaration muss nicht vorhanden sein */
typedef void (i8086DeviceReset)();

/* Speicher an eine freie Stelle in buf das Devicehandle */
/* handle: DLL-Handle vom Typ *void                      */
/* buf: Array of *void                                   */
#define i8086SAVE_DEVICE_HANDLE(handle, buf) { int i; \
    for (i=0; 1; i++) \
      if (!buf[i]) {buf[i]=handle; break;} }

int i8086LoadDevice(const char *filename);
int i8086LoadAllDevices(i8086core *core);
void i8086CloseAllDevices();
void i8086ResetAllDevices();

#endif /* _i8086DEVICES_H_ */
