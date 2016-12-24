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
/* Funktionen für die Nachrichtenverwaltung.        */
/*                                                  */
/****************************************************/

#ifndef _i8086MESSAGES_H_
#define _i8086MESSAGES_H_

#include <sys/sem.h>
#include <sys/ipc.h>
#include "i8086proc.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* Signal zum ausloesen eines Interrupt */
/* hParam = Int-Nummer                  */
#define i8086_SIG_CALL_INT 0

/* Signal fuer Port wurde gesetzt */
/* hParam = Portnummer            */
#define i8086_SIG_PORT_OUT 1
#define i8086_SIG_PORT_IN 5

/* Schreibt einen Wert in einen Port */
/* hParam = Portnummer */
/* lParam = Value */
#define i8086_SIG_PORT_WRITE_VALUE 2

/* Wird nach jedem Clock aufgrufen  */
/* hParam = bereits eingefuegte INT */
/*          erster Aufruf mach proc */
/*          Bei Kaskadierten Aufrufen muss hParam in der */
/*          der jeweiligen Fkt erhoeht werden.           */
/* lParam = aktuelle Flags des Prozessors                */
#define i8086_SIG_INT_REQUEST 3

/* Signal das ein Geraet einen IRQ ausgeloesst hat */
/* hParam = IRQ-Maske                              */
#define i8086_SIG_IRQ 4
/*  Einzelne IRQs koennen kombiniert werden        */
    #define i8086_IRQ_0 1
    #define i8086_IRQ_1 2
    #define i8086_IRQ_2 4
    #define i8086_IRQ_3 8
    #define i8086_IRQ_4 16
    #define i8086_IRQ_5 32
    #define i8086_IRQ_6 64
    #define i8086_IRQ_7 128

/* Schreibt ein Byte in den Speicher */
/* hParam = Adresse                  */
/* lParam = value                    */
#define i8086_SIG_WMEM 6    

/* Schreibt einen Block Bytes in den Speicher */
/* hParam = In Byte 0-19 befindet sich die    */
/*          Anfangsadresse ab der geschrieben */
/*          werden soll.
/*          In Byte 20-31 die Anzahl der Bytes*/
/*          die geschrieben werden (max. 4095)*/ 
/* lParam = Anfangsadresse des Lesespeichers. */
#define i8086_SIG_WBMEM 7

/* Wird vom i8086_SIG_WMEM-Handler gesendet, */
/* nachdem der Speicher kopiert wurde.       */
/* hParam = hParam von i8086_SIG_WMEM        */
/* lParam = lParam von i8086_SIG_WMEM        */
#define i8086_SIG_WBMEM_EXEC 8

/* Signal fuer eigene Nachrichten */
#define i8086_SIG_USER 15

/* Max. Anzahl verschiedener Msgs */
#define i8086_MAX_MSG 32
/* Max. Laenge der MsgQueue */
#define i8086_MSG_QUEUE_LEN 64
/* Semephorenschluessel */
#define i8086_SEM_KEY 14211

typedef void (i8086msgFunc)(unsigned short msg, unsigned int hParam, unsigned int lParam);
typedef i8086msgFunc* (i8086PSetMsgFunc)(unsigned short msg, i8086msgFunc *msgFunc);
typedef void (i8086msgLoop)(unsigned int msg, unsigned int hParam, unsigned int lParam);


/* Nachricht */
typedef struct i8086Msg
{
  unsigned int msg; /* Nachrichtennummer   */
  unsigned int hParam;  /* Daten der Nachricht */
  unsigned int lParam;  /* Daten der Nachricht */
}i8086Msg;

/* Einzelnes Listenelement der MsgQueue */
typedef struct i8086MsgListElement
{
  struct i8086MsgListElement *next;
  struct i8086MsgListElement *prev;
  i8086Msg *msg;
}i8086MsgListElement;

/* MsgQueue-Header */
typedef struct i8086MsgListHeader
{
  struct i8086MsgListElement *first;
  struct i8086MsgListElement *last;
  unsigned long length;
}i8086MsgListHeader;

i8086MsgListHeader *MsgListHeader;
i8086msgFunc **msgFuncs;
#ifdef _WIN32
LPCRITICAL_SECTION criticalSection;
#else
key_t criticalSection;
#endif

/* Liefert einen Int in dem in den Bytes 0-19 adr und */
/* 20-31 count steht.                                 */
#define GetAdrAndCount(adr, count) ((unsigned int)( (adr & 4194303) | ((count & 4095)<<20) ))

/* Fkts/Makros fuer Semaphoren */
#define P(id) pv(id, -1)
#define V(id) pv(id, 1)

/* Makros fuer die Versendung von Nachrichten     */
/*  Fuegt eine Nachricht aus einem anderen        */
/*  Thread in die MsgQueue ein.                   */
/*  msg: unsigned short - Nachrichtennummer       */
/*  hParam: unsigned int                          */
/*  lParam: unsigned int                          */
/*  semId: Semaphorenid                           */
/*  sendFct: i8086msgFunc - Nachrichtensende-fkt. */
// #define i8086SendMsg(msg, hParam, lParam, semId, sendFct) { P(semId); sendFct(msg, hParam, lParam); V(semId); }
/*  Fuegt eine Nachricht in die MsgQueue ein.     */
/*  msg: unsigned short - Nachrichtennummer       */
/*  hParam: unsigned int                          */
/*  lParam: unsigned int                          */
// #define i8086SendMsgProc(msg, hParam, lParam) { P(criticalSection); i8086PushMsg(msg, hParam, lParam); V(criticalSection); }

void i8086initMsgQueue();

/*  Fuegt eine Nachricht in die MsgQueue ein.     */
/*  msg: unsigned short - Nachrichtennummer       */
/*  hParam: unsigned int                          */
/*  lParam: unsigned int                          */
void i8086PushMsg(unsigned short msg, unsigned int hParam, unsigned int lParam);
i8086Msg* i8086PopMsg();
i8086msgFunc* i8086SetMsgFunc(unsigned short msg, i8086msgFunc *msgFunc);
void i8086ClearMsgQueue();
void i8086ProcessMsgQueue(i8086core *core);
void i8086CloseMsgQueue();
#ifdef _WIN32
void pv(LPCRITICAL_SECTION id, int operation);
#else
void pv(int id, int operation);
#endif


#endif /* _i8086MESSAGES_H_ */
