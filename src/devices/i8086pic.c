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
/* PIC                                              */
/*                                                  */
/****************************************************/

#include "../i8086devices.h"
#include "../i8086error.h"
#include "../i8086util.h"

i8086msgFunc *oldPortOutHandler, *oldIntRequestHandler, *oldIRQRequestHandler;
i8086DeviceCalls *funcCalls;
unsigned char state=0, INTOffset=0, IMR;
unsigned char AEOI, IRR=0, ISR=0x0;
char lastActiveIRQ=-1;


void picPortOutHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  unsigned char A0=0;
  
  if (hParam==0xc0 || hParam==0xc2)
  {
    if (state==4)
      state = 5;
      
    if (hParam==0xc2) /* 9Bit vom Instruction-Word */
      A0=1;
      
    if (A0==0 && (lParam&16)) /* Power up, ICW1 wird bearbeitet */
    {
      if (lParam & 1)   /* ICW4 wird benoetigt */
        if (lParam & 2) /* SingleMode (nicht Kaskadiert) */
          if (lParam & 16) /* const 1 */
            state=1;
    } 
       
    if (state==1 && A0==1) /* ICW2, Offset fuer IRQ -> INT */
    {
      INTOffset=lParam & 0xF8;
      state=2;
    }
    else /* ICW3 kommt nur bei Kaskadenmodus */
      if (state==2 && A0==1) /* ICW4 */
      {
        if (lParam & 1) /* 8086-Modus */
          if ((lParam & 0xF8)==0) /* Bit3-7 muessen 0 sein */
            state=4;
        if (lParam & 2)
          AEOI=1;
        else
          AEOI=0;
          
        if (state!=4)
          i8086warning("PIC - Unsupported Mode.");
      }
    
    if (state==5)
    {
      if (A0==1) /* OCW1 */
      {
        IMR=lParam;
        funcCalls->SendMsg(i8086_SIG_PORT_WRITE_VALUE, 0x2c, lParam); 
      }
      else
      {
        if ((lParam & 0x18)==0) /* OCW2, Bit3-4 muessen 0 sein */
        {
          unsigned char bits;
          
          bits = getBitSnipped((char)lParam, 7, 3);
          if (bits==1) /* zuletzt aktivierter IRQ wird wieder aktiviert */
            if (lastActiveIRQ>=0)
              ISR = ISR & (~(1<<lastActiveIRQ));  
        }
      }
    }
  }

  if (oldPortOutHandler)
    oldPortOutHandler(msg, hParam, lParam);
}

void picIntRequestHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  int i=0, hasSend=0;

    //wir brauchen endlich lesezugriff auf core!, sonst is man nur noch am Nachrichten schicken
    //direkte IF Abfrage im PIC ist zwar unrealistisch, aber emuliert am ehesten das IRQ Request und Ack verhalten der CPU
    //wenn die CPU keine IRQ akzeptiert dann verfällt der IRC uch nicht
  if (state==5 && (lParam & i8086_FLAG_IF))
    while (IRR)
    {
      if (IRR & (1<<i))
      {
        if (!(ISR & (1<<i)))
        {
          funcCalls->SendMsg(i8086_SIG_CALL_INT, i+INTOffset, 0);
          hasSend=1;
        }
        IRR = IRR & (~(1<<i));
        ISR = ISR | (1<<i);
        if (AEOI) /* Automodus */
          ISR = ISR & (~(1<<i));
        lastActiveIRQ = i;
        break;
      }
      i++;
    }
  
  if (oldIntRequestHandler)
    oldIntRequestHandler(msg, hParam+hasSend, lParam);
}

void picIRQRequestHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  IRR = IRR | (char)hParam;
  IRR = IRR & (~IMR);

  if (oldIRQRequestHandler)
    oldIRQRequestHandler(msg, hParam, lParam);
}

void DeviceInit(i8086DeviceCalls *calls)
{
  funcCalls = calls;
  oldPortOutHandler = funcCalls->SetMsgHandler(i8086_SIG_PORT_OUT, picPortOutHandler);
  oldIntRequestHandler = funcCalls->SetMsgHandler(i8086_SIG_INT_REQUEST, picIntRequestHandler);
  oldIRQRequestHandler = funcCalls->SetMsgHandler(i8086_SIG_IRQ, picIRQRequestHandler);
}
