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
/* Schreibt in den Speicher was es will.            */
/*                                                  */
/****************************************************/

#include <stdlib.h>
#include <time.h>
#include "../i8086util.h"
#include "../i8086devices.h"
#include "../i8086error.h"


i8086DeviceCalls *funcCalls;

void DeviceClose()
{

}

void DeviceInit(i8086DeviceCalls *calls)
{
  #define m_size 4000
  char *gore;
  int i;
  
  funcCalls = calls;
  gore = (char*)malloc(m_size);
  srand(time(NULL));
  for (i=0; i<m_size; i++)
    gore[i] = (char unsigned)rand();
  funcCalls->SendMsg(i8086_SIG_WBMEM, GetAdrAndCount(0x1000, m_size), (unsigned int)gore);
}
