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
/* Autor: CST                                       */
/* Version: 1.0                                     */
/* ** => nicht beendet                              */
/*                                                  */
/* PC-Speaker   Frequenz an port 0xff + Tonlaenge an port 0xef                                        */
/*                                                  */
/****************************************************/

#include "../i8086devices.h"
#include "../i8086error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../i8086util.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

i8086msgFunc *oldPortOutHandler;
i8086DeviceCalls *funcCalls;
int stdlength=200;

void beepPortOutHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  char freq[6],len[6],*cmd;
  int res;
  
  if (hParam==0xef)
  {
    stdlength=lParam;
  }
  
  if (hParam==0xff && lParam<20000 && res>=0)
  {
    res=sprintf(freq,"%d",lParam);
    res=sprintf(len,"%d",stdlength);
    cmd=(char*)calloc(1, sizeof(char));
    cmd=stringAdd(cmd,"/usr/bin/beep -f ",freq," -l ",len,NULL);
           #ifdef _WIN32
                 Beep(lParam, stdlength);
           #else
                system(cmd);
           #endif
     free(cmd);
  }
  
  if (oldPortOutHandler)
    oldPortOutHandler(msg, hParam, lParam);
}


void DeviceClose()
{

}

void DeviceInit(i8086DeviceCalls *calls)
{
  funcCalls = calls;
  oldPortOutHandler = funcCalls->SetMsgHandler(i8086_SIG_PORT_OUT, beepPortOutHandler);
}
