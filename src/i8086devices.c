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

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "i8086devices.h"
#include "i8086messages.h"
#include "i8086error.h"
#include "i8086config.h"
#include "i8086util.h"
#include "i8086emufuncs.h"

int i8086CheckDllError();


unsigned char i8086MemReadByteWrapper(i808616BitAdr adr, i808616BitAdr seg)
{
  return currentCore->mem[adr+(seg*0x10)];
}

unsigned short i8086MemReadWordWrapper(i808616BitAdr adr, i808616BitAdr seg)
{
  unsigned short w=0;
  
	w=w|currentCore->mem[adr+(seg*0x10)];
	w=w|(currentCore->mem[adr+(seg*0x10)+1]<<8);
	return w;
}


int i8086LoadDevice(const char *filename)
{
  void *handle;
  i8086DeviceInit *init;
  i8086DeviceCalls *calls;
  int ret=i8086_SUC_DEV_LOAD;

  if ( (handle = dlopen(filename, RTLD_NOW))!=NULL )
  {
    init = (i8086DeviceInit*)dlsym(handle, "DeviceInit");
    ret=i8086CheckDllError();
    i8086SAVE_DEVICE_HANDLE(handle, DeviceHandles);
    if (ret==i8086_SUC_DEV_LOAD && init!=NULL)
    {
      calls = (i8086DeviceCalls*)calloc(1, sizeof(i8086DeviceCalls));
      calls->Version = VERSION_NUMBER; /* VERSION_NUMBER = Define in Makefile */
      calls->SetMsgHandler = i8086SetMsgFunc;
      calls->SendMsg = i8086PushMsg;
      calls->MemReadByte = i8086MemReadByteWrapper;
      calls->MemReadWord = i8086MemReadWordWrapper;
      init(calls);
    }
      
    //dlclose(handle);
    //ret=i8086CheckDllError();
  }
  else
    ret=i8086CheckDllError();
    
  return ret;
}

/* Lädt alle Geräte die in der Config-Datei stehen. */
/* returns: Anzahl der erfolgreich geladenen Geräte */
int i8086LoadAllDevices(i8086core *core)
{
  int devcount, i, loaded=0;
  
  i8086warning(i8086_STR_LOAD_DEVICES);
  
  if (i8086ConfigExists(CONFIG_FILE)==i8086_ERR_FILENF)
  {  
    i8086warning(i8086_ERR_STR_CONFIGNF);
    return loaded;
  }
  
  devcount = i8086ReadDecConfig(CONFIG_FILE, "DEVICECOUNT", 0);
  currentCore = core;

  DeviceHandles = (void**)calloc(devcount+1, sizeof(void*));
    
  for (i=0; i<devcount; i++)
  {
    char cfgStr[i8086_CFG_MAX_VALUE_LEN];
    char devName[10];
    
    sprintf(devName, "DEVICE%d", i);
    
    if (i8086ReadStrConfig(cfgStr, CONFIG_FILE, devName)!=0)
    {
      if (i8086LoadDevice(cfgStr)==i8086_ERR_DEV_LOAD)
      {
        strcat(cfgStr, ": ");
        strcat(cfgStr, i8086_ERR_STR_FILENF);
        i8086warning(cfgStr);
      }
      else
      {
        strcat(cfgStr, ": ");
        strcat(cfgStr, i8086_STR_FILELOAD);
        i8086warning(cfgStr);
        loaded++;
      }
    }
  }
  
  return loaded;
}

void i8086CloseAllDevices()
{
  int i, ret=i8086_SUC_DEV_LOAD;
  i8086DeviceClose *devclose;
  
  i8086warning(i8086_STR_CLOSE_DEVICES);
  
  if (DeviceHandles)
  {
    for (i=0; DeviceHandles[i]!=NULL; i++)
    {
      devclose = (i8086DeviceClose*)dlsym(DeviceHandles[i], "DeviceClose");
      ret=i8086CheckDllError();
      if (ret==i8086_SUC_DEV_LOAD && devclose!=NULL)
      {
        devclose();
      }
      dlclose(DeviceHandles[i]);
      i8086CheckDllError();
    }
    free(DeviceHandles);
    DeviceHandles=NULL;
  }
}

/* Ruft in allen geladenen Geraeten die */
/* Reset-Fkt auf.                       */
void i8086ResetAllDevices()
{
  int i, ret;
  i8086DeviceReset *devreset;
  
  if (DeviceHandles)
  {
    for (i=0; DeviceHandles[i]!=NULL; i++)
    {
      devreset = (i8086DeviceClose*)dlsym(DeviceHandles[i], "DeviceReset");
      ret=i8086CheckDllError();
      if (ret==i8086_SUC_DEV_LOAD && devreset!=NULL)
        devreset();
    }
  }  
}

int i8086CheckDllError()
{
  char *error = dlerror();
  
  if (error != NULL) 
  {
    i8086warning(error);
    return i8086_ERR_DEV_LOAD;
  }
  return i8086_SUC_DEV_LOAD;
}

