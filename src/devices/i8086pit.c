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
/* PIT                                              */
/*                                                  */
/****************************************************/

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "../i8086devices.h"
#include "../i8086error.h"
#include "../i8086util.h"
#include "i8086pit.h"

i8086DeviceCalls *funcCalls;
i8086msgFunc *oldPitIntRequestHandler, *oldPitPortOutHandler;
unsigned int clocks, releaseIRQ[6] = {0, 0, 0, 0, 0, 0};
char stopThread=0;
unsigned char ControlWordRegister[] = {0, 0, 0};
int workCounter[] = {0, 0, 0};
unsigned short Counter[] = {0, 0, 0};
char currentCounter=-1, setCounterValueState=0;
pthread_t pit_thread;


void pitPortOutHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  if (hParam==0x0a6) /* CONTROL-BYTE */
  {
    currentCounter = getBitSnipped((char)lParam, 7, 2); /* gewaelter Counter */
    if (currentCounter<2)
    {
      ControlWordRegister[(unsigned short)currentCounter] = (char)lParam;
      Counter[(unsigned short)currentCounter] = 0;
      workCounter[(unsigned short)currentCounter] = 0;
      setCounterValueState = 0;
    }
  }
  
  if (hParam==0x0a2) /* Counter-Value */
  {
    if (currentCounter>=0)
    {
      if (setCounterValueState==0) /* Low-Value */
      {
        Counter[(unsigned short)currentCounter] = lParam;
        setCounterValueState=1;
      }
      else
        if (setCounterValueState==1) /* High-Value */
        {
          Counter[(unsigned short)currentCounter] = Counter[(unsigned short)currentCounter] + (lParam<<8);
          workCounter[(unsigned short)currentCounter] = Counter[(unsigned short)currentCounter];
          setCounterValueState = 2;
        }
    }
  }

  if (oldPitPortOutHandler)
    oldPitPortOutHandler(msg, hParam, lParam);
}

void *pitThreadFunction(void *arg) 
{
  struct timespec req;
  unsigned char i;
  
  #ifdef MAX_DEBUG
  i8086warning("PIT-Thread started.");
  #endif
  
  req.tv_sec=0;
  req.tv_nsec=100000;
  
  while (!stopThread)
  {
    //usleep(10000); /* 10 msec */
    delay(10000);  /* 10 msec */
    //nanosleep(&req, &rem);
    for (i=0; i<3; i++) /* Counter setzen */
    {
      if (Counter[i]>0)
      {
        workCounter[i] = workCounter[i]-18432;//(int)(rem.tv_nsec*0.0018432); 
        if (workCounter[i]<=0)
        {
          workCounter[i] = Counter[i];
          switch (i)
          {
            case PIT_CURRENT_IRQS_0 : releaseIRQ[PIT_CURRENT_IRQS_0]++;
                                     break;
            case PIT_CURRENT_IRQS_1 : releaseIRQ[PIT_CURRENT_IRQS_1]++;
                                     break;
            case PIT_CURRENT_IRQS_2 : releaseIRQ[PIT_CURRENT_IRQS_2]++;
                                     break;
          }
        }
      }
    }
  }
  
  #ifdef MAX_DEBUG
  i8086warning("PIT-Thread end.");
  #endif
  pthread_exit("");
  
  return 0;
}


void pitIntRequestHandler(unsigned short msg, unsigned int hParam, unsigned int lParam)
{  
  /* IRQ0, Timer1 */
  if (releaseIRQ[PIT_CURRENT_IRQS_1] != releaseIRQ[PIT_LAST_IRQS_1])
  {
    funcCalls->SendMsg(i8086_SIG_IRQ, i8086_IRQ_0, 0);
    releaseIRQ[PIT_LAST_IRQS_1] = releaseIRQ[PIT_CURRENT_IRQS_1];
  }
  /* IRQ1, Timer2 */
  if (releaseIRQ[PIT_CURRENT_IRQS_2] != releaseIRQ[PIT_LAST_IRQS_2])
  {
    funcCalls->SendMsg(i8086_SIG_IRQ, i8086_IRQ_1, 0);
    releaseIRQ[PIT_LAST_IRQS_2] = releaseIRQ[PIT_CURRENT_IRQS_2];
  }

  if (oldPitIntRequestHandler)
    oldPitIntRequestHandler(msg, hParam, lParam);
}

void pitCreateThread()
{
    int res;
    //pthread_t pit_thread;

    res = pthread_create(&pit_thread, NULL, pitThreadFunction, (void*)"");
    if (res != 0) 
    {
      #ifdef MAX_DEBUG
      i8086warning("PIT-Thread creation failed");
      #endif
    }
}

void DeviceInit(i8086DeviceCalls *calls)
{
  funcCalls = calls;
  
  oldPitPortOutHandler = funcCalls->SetMsgHandler(i8086_SIG_PORT_OUT, pitPortOutHandler);
  oldPitIntRequestHandler = funcCalls->SetMsgHandler(i8086_SIG_INT_REQUEST, pitIntRequestHandler);
  
  pitCreateThread();
}

void DeviceClose()
{
  void *thread_result;
  int res;
  
  #ifdef MAX_DEBUG
  i8086warning("Waiting for PIT-Thread.");
  #endif
  stopThread=1;
  //if (pit_thread)
  res = pthread_join(pit_thread, &thread_result);
  #ifdef MAX_DEBUG
  if (res != 0) 
    i8086warning("Waiting for PIT-Thread failed.");
  else
    i8086warning("PIT-Thread closed.");
  #endif
}

void DeviceReset()
{
  int i;
  
  for (i=0; i<3; i++)
  {
    workCounter[i] = 0;
    Counter[i] = 0;
  }
}
