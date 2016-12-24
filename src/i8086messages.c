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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "i8086messages.h"
#include "i8086error.h"


i8086MsgListHeader *newi8086MsgList();
void i8086MsgListInsertElementAtTop(i8086MsgListHeader *head, i8086MsgListElement *element);
i8086Msg* i8086MsgListDeleteFromBottom(i8086MsgListHeader *head);

/* Initialisiert den Header und MsgFuncs-Array */
void i8086initMsgQueue()
{
  #ifdef _WIN32
  criticalSection = (LPCRITICAL_SECTION)calloc(1, sizeof(CRITICAL_SECTION));  
  InitializeCriticalSection(criticalSection);
  #else
  if ((criticalSection = semget(i8086_SEM_KEY, 1, S_IRWXU | S_IRWXG | S_IRWXO | IPC_CREAT | IPC_EXCL))==-1) // S_IRWXU | S_IRWXG | S_IRWXO |
    if (errno==EEXIST)
    {
      criticalSection = semget(i8086_SEM_KEY, 0, 0);
      semctl(criticalSection, 0, IPC_RMID);
      if ((criticalSection = semget(i8086_SEM_KEY, 1, S_IRWXU | S_IRWXG | S_IRWXO | IPC_CREAT))==-1)
        i8086warning(i8086_ERR_OPEN_SEM);
    }
  if (semctl(criticalSection, 0, SETVAL, (int)1)==-1)
    i8086warning(i8086_ERR_INIT_SEM);
  #endif
  MsgListHeader = newi8086MsgList();
  msgFuncs = (i8086msgFunc**)calloc(i8086_MAX_MSG, sizeof(i8086msgFunc*));
}

/* Fuegt eine neue Msg ein */
void i8086PushMsg(unsigned short msg, unsigned int hParam, unsigned int lParam)
{
  i8086MsgListElement *e;
  i8086Msg *m;
  
  assert(MsgListHeader!=NULL); 
  
  //#ifdef _WIN32
  //EnterCriticalSection(&criticalSection);
  //#else
  //P(criticalSection);
  //#endif
  if (MsgListHeader->length <= i8086_MSG_QUEUE_LEN) /* Max. Laenge der Queue pruefen */
  {
    P(criticalSection);
    e = (i8086MsgListElement*)malloc(sizeof(i8086MsgListElement));
    m = (i8086Msg*)malloc(sizeof(i8086Msg));
    m->msg = msg;
    m->hParam = hParam;
    m->lParam = lParam;
    e->msg = m;
    e->next = NULL;
    e->prev = NULL;
    i8086MsgListInsertElementAtTop(MsgListHeader, e);
    V(criticalSection);
  }
  
  //if (MsgListHeader->length <= i8086_MSG_QUEUE_LEN) /* Max. Laenge der Queue pruefen */
    //i8086MsgListInsertElementAtTop(MsgListHeader, e);
  //#ifdef _WIN32
  //LeaveCriticalSection(&criticalSection);
  //#else
  //V(criticalSection);
  //#endif
}

/* Entfernt die aelteste Msg und gibt sie zurueck. */
/* Sind keine weiteren Nachrichten in der Liste    */
/* wird NULL zurückgegeben.                        */
i8086Msg* i8086PopMsg()
{
  return i8086MsgListDeleteFromBottom(MsgListHeader);
}

/* Setzt die Funktion welche beim Auftreten von msg    */
/* aufgerufen werden soll.                             */
/* Gibt die Funktion zurueck die zuvor mit dieser msg  */
/* verknuepft war. War keine Funktion mit dieser msg   */
/* verknuepft oder ist die msg auserhalb des gueltigen */
/* Wertes wird NULL zurueckgegeben.                    */
i8086msgFunc* i8086SetMsgFunc(unsigned short msg, i8086msgFunc *msgFunc)
{
  i8086msgFunc* f=NULL;
  
  if (msg>=i8086_MAX_MSG)
    return f;
    
  f = msgFuncs[msg];
  msgFuncs[msg] = msgFunc;
  
  return f;
}

/* Fuehrt alle Nachrichten aus und loescht sie danach. */
void i8086ProcessMsgQueue(i8086core *core)
{
  i8086Msg* msg;

  msg = i8086PopMsg();
    
  while (msg!=NULL)
  {
    if (msgFuncs[msg->msg] != NULL)
      msgFuncs[msg->msg](msg->msg, msg->hParam, msg->lParam);
    free(msg);
    msg = i8086PopMsg();
  }
  /* Interrupt-Freigabe */
  if (msgFuncs[i8086_SIG_INT_REQUEST] != NULL)
    msgFuncs[i8086_SIG_INT_REQUEST](i8086_SIG_INT_REQUEST, 0, core->flags);

}

/* Entfernt alle Nachrichten aus der Schlange */
/* ohne sie auszufuehren.                     */
void i8086ClearMsgQueue()
{
  i8086Msg* msg;
    
  while ((msg=i8086PopMsg())!=NULL)
    free(msg);  
}

/* Schliesst IPC */
void i8086CloseMsgQueue()
{
  /* MsgQueue leeren */
  i8086ClearMsgQueue();


#ifndef _WIN32
  semctl(criticalSection, 0, IPC_RMID);
#else
  //if (criticalSection!=NULL)
  DeleteCriticalSection(criticalSection);
  free(criticalSection);
#endif
  free(MsgListHeader);
  free(msgFuncs);
}

/* Realisiert P- und V-Opertation fuer Semaphoren. */
/* id: Schluessel der Semaphore                    */
/* operation: Wert um der die Semaphore erhoet     */
/*            werden soll.                         */
#ifdef _WIN32
void pv(LPCRITICAL_SECTION id, int operation)
{
  if (operation<0)
    EnterCriticalSection(id);
  else
    if (operation>0)
      LeaveCriticalSection(id);
}
#else
void pv(int id, int operation)
{
  static struct sembuf sem;
  
  sem.sem_op = operation;
  sem.sem_flg = SEM_UNDO;
  #ifdef MAX_DEBUG
  if (semop(id, &sem, 1)==-1)
    i8086warning(i8086_ERR_PV_SEM);
  #endif /* MAX_DEBUG */
}
#endif /* _WIN32 */

/***************************Listenfunktionen**********************************************/

/* Erstellt einen neuen Listen-Header */
i8086MsgListHeader *newi8086MsgList()
{
  i8086MsgListHeader *head = (i8086MsgListHeader*)malloc(sizeof(i8086MsgListHeader));
  assert(head!=NULL);
  head->first = NULL;
  head->last = NULL;
  head->length = 0;
  return head;
}

/* Fuegt die Adresse eines sListElement an den Anfang der Liste ein */
void i8086MsgListInsertElementAtTop(i8086MsgListHeader *head, i8086MsgListElement *element)
{
  assert(head!=NULL);
  assert(element!=NULL);
/*
#ifdef _WIN32
  EnterCriticalSection(criticalSection);
#else
  P(criticalSection);  
#endif
*/
  if (head->first==NULL) /* Liste ist leer */
  {
    head->first = element;
    head->last = element;
    //element->next = NULL;
  }
  else
  {
    head->first->prev = element;
    element->next = head->first;
    head->first = element;
  }
  head->length++;
/*  
#ifdef _WIN32
  LeaveCriticalSection(criticalSection);
#else
  V(criticalSection);
#endif */
}

/* Loescht das letzte MsgQueue-Element und gibt die Msg zurueck.  */
/* Wenn die MsgQueue leer ist wird NULL zurueck gegeben.          */
/* Der Speicher der zurueckgegebenen Msg muss freigegeben werden. */
i8086Msg* i8086MsgListDeleteFromBottom(i8086MsgListHeader *head)
{
  i8086Msg *e=NULL;
  i8086MsgListElement *l;
  
  if (head->length>0)
  {
    //#ifdef _WIN32
    //EnterCriticalSection(&criticalSection);
    //#else
    P(criticalSection);
    //#endif

    e = head->last->msg;
    l = head->last;
    head->last = head->last->prev;
    head->length--;
    free(l);
    if (head->length==0)
      head->first = NULL;
    else
      head->last->next = NULL;
      
    //#ifdef _WIN32
    //LeaveCriticalSection(&criticalSection);
    //#else
    V(criticalSection);
    //#endif
  }
  
  return e;
}
