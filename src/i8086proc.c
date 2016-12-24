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

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "i8086proc.h"
#include "i8086error.h"
#include "i8086emufuncs.h"
#include "i8086util.h"
#include "i8086messages.h"
#include "i8086controltrans.h"
#include "i8086devices.h"
#include "i8086config.h"

/* Uebersetzung von Registercode(Word) in Register-Array-Pos */
unsigned char transW[8] = {i8086_AR_AX, i8086_AR_CX, i8086_AR_DX, i8086_AR_BX, i8086_AR_SP, i8086_AR_BP, i8086_AR_SI, i8086_AR_DI};
/* Uebersetzung von Registercode(Byte) in Register-Array-Pos */
unsigned char transB[8] = {i8086_AR_AL, i8086_AR_CL, i8086_AR_DL, i8086_AR_BL,
                           i8086_AR_AH, i8086_AR_CH, i8086_AR_DH, i8086_AR_BH};

                           
/* Signalhandler fuer das Schreiben auf einen Port */
void writePortValueSignalHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  core->ports.x[hParam]=lParam;
}                           

/* Signalhandler fuer Interrupt */
void IntSignalHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  i8086callInterrupt(core, hParam);
}

/* Signalhandler fuer MemWrite */
void WMemSignalHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  core->mem[hParam] = (unsigned char)lParam;
}

/* Signalhandler fuer Block-MemWrite */
/* Danger!                           */
void WBMemSignalHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  char *r_mem;
  int size, w_pos, i;
  
  r_mem = (char*)lParam;
  size = getBitSnippedInt(hParam, 31, 12);
  w_pos = getBitSnippedInt(hParam, 19, 20);
  for (i=0; i<size; i++)
    core->mem[w_pos+i] = (unsigned char)(r_mem[i]);
    
  i8086PushMsg(i8086_SIG_WBMEM_EXEC, hParam, lParam);
}

/* Register sezten. w=1 -> 16b, w=0 -> 8b. siehe 231456.pdf Seite 30 */
/* reg    Register-Konstante                                         */
/* value  Neuer Wert des Registers                                   */
void i8086SetRegister(i8086core *core, int reg, i8086W w, unsigned short value)
{
  if (w)
  {
    core->reg.x[transW[reg]] = value;
  }
  else
  {
    core->reg.b[transB[reg]] = (unsigned char)value;
  }
}

/* Register lesen. w=1 -> 16b, w=0 -> 8b. siehe 231456.pdf Seite 30    */
/* reg  Register-Konstante                                             */
/* 8Bit-Register stehen immer im l-Byte des zurueckgegebenen Registers */
/* Returns: der gelesene Wert steht in buf                             */
void i8086GetRegisterRef(i8086SingleRegister *buf, i8086core *core, i8086W w, int reg)
{
  buf->x = 0;
  if (w)
    buf->x = core->reg.x[transW[reg]];
  else
    buf->b[0] = core->reg.b[transB[reg]];  
}

/* Register lesen. w=1 -> 16b, w=0 -> 8b. siehe 231456.pdf Seite 30    */
/* reg  Register-Konstante                                             */
/* 8Bit-Register stehen immer im l-Byte des zurueckgegebenen Registers */
i8086SingleRegister* i8086GetRegister(i8086core *core, i8086W w, int reg)
{
  i8086SingleRegister *r = (i8086SingleRegister*)malloc(sizeof(i8086SingleRegister));

  i8086GetRegisterRef(r, core, w, reg);

  return r;
}

/* Setzt den Wert eines Segment-Registers.       */
/* reg gibt das Register an (siehe: i8086proc.h) */
/* value wird in das Register eingetragen        */
void i8086SetSegRegister(i8086core *core, int reg, unsigned short value)
{
  core->reg.x[i8086_FIRST_SEGREG+reg] = value;
}

/* Gibt den Wert eines Segment-Registers zurueck.      */
/* reg gibt das Register an (siehe: i8086proc.h)       */
/* Ist convert ungleich 0, gibt die Funktion einen     */
/*     bereits umgerechneten Wert zurueck (SegReg*10). */
/* Ist convert 0, wird der Inhalt des Registers        */
/*     unveraendert zurueckgegeben.                    */
i8086SingleRegister* i8086GetSegRegister(i8086core *core, int reg, int convert)
{
  #define SEG_STEP 0x10

 i8086SingleRegister *r = (i8086SingleRegister*)malloc(sizeof(i8086SingleRegister));

  r->x = core->reg.x[i8086_FIRST_SEGREG+reg];
  if (convert!=0)
    r->x = r->x*SEG_STEP;

  return r;
}

/* Gibt den Wert eines Segment-Registers zurueck.      */
/* reg gibt das Register an (siehe: i8086proc.h)       */
/* Ist convert ungleich 0, gibt die Funktion einen     */
/*     bereits umgerechneten Wert zurueck (SegReg*10). */
/* Ist convert 0, wird der Inhalt des Registers        */
/*     unveraendert zurueckgegeben.                    */
unsigned int i8086GetSegRegister_fast(i8086core *core, int reg, int convert)
{
  #define SEG_STEP 0x10
  if (convert)
      return core->reg.x[i8086_FIRST_SEGREG+reg]*SEG_STEP;
  else
    return core->reg.x[i8086_FIRST_SEGREG+reg];
}

/* Setzt die Handler fuer die internen Ereignisse */
void i8086initHandler()
{
  i8086SetMsgFunc(i8086_SIG_PORT_WRITE_VALUE, writePortValueSignalHandler);
  i8086SetMsgFunc(i8086_SIG_CALL_INT, IntSignalHandler);
  i8086SetMsgFunc(i8086_SIG_WMEM, WMemSignalHandler);
  i8086SetMsgFunc(i8086_SIG_WBMEM, WBMemSignalHandler);
}

/* Initialisierung des core, des command-Arrays und der MsgQueue */
/* Für mem wird (char* x i8086_MEM_SIZE * ADDMEMSEGMENTS) reserviert              */
/* Für commands wird (256 x *i8086command) reserviert            */
void i8086init()
{
  int i, addSegs=1;
  
  core = (i8086core*)malloc(sizeof(i8086core));
  /*
  addSegs += i8086ReadDecConfig(CONFIG_FILE, "ADDMEMSEGMENTS", i8086_MAX_MEM_SEG);
  if (addSegs>i8086_MAX_MEM_SEG) 
    addSegs = i8086_MAX_MEM_SEG;
  core->mem = (unsigned char*)calloc(i8086_MEM_SIZE*addSegs, sizeof(unsigned char));
  */
  core->mem = (unsigned char*)calloc((i8086_MEM_SIZE*i8086_MAX_MEM_SEG), sizeof(unsigned char));
  
  for (i=0; i<13; i++)
    core->reg.x[i] = 0;
  core->flags = i8086_CPU_SIG; //cpu ist ein 8086, die höchsten 4 bit müssen 1 sein
  core->reg.x[i8086_AR_SP] = i8086_BEGINADR;
  for(i=0;i<i8086_PORTS*2;i++)
    core->ports.x[i]=0;

  commands = (i8086command**)calloc(256, sizeof(i8086command*));
  core->miscData.processedClocks = 0;
  core->miscData.emuTime = 0;
  core->miscData.segoverride = -1;

  i8086InitCommands(commands);
  i8086initMsgQueue();
  i8086initHandler();
  i8086LoadAllDevices(core); /* Geraete laden */
}

/* Gibt die vom Emu verwendeten Ressourcen frei */
void i8086close()
{
  free(commands);
  free(core->mem);
  free(core);
  i8086CloseAllDevices();
  i8086CloseMsgQueue();
}

/* Prozessor-Reset  */
/* Alle Register werden auf 0 gesetzt.                */
/* Aufruf der Reset-Fkt. in allen geladenen Geraeten. */
void i8086reset()
{
  int i;
  
  for (i=0; i<13; i++)
    core->reg.x[i] = 0;

  i8086ResetAllDevices();
  i8086ClearMsgQueue();
}

/* Lädt com-Datei in Speicher                                   */
/* Return: Gibt im Erfolgsfall i8086_SUC_FILELOAD               */
/*         Datei konnte nicht geladen werden i8086_ERR_FILENF   */
/*         Datei konnte nicht gelesen werden i8086_ERR_FILEREAD */
int i8086loadBinFile(i8086core* core, const char* filename, unsigned int beginAdr)
{
  FILE *file;
  int r=0;
  unsigned short cs;

  file = fopen(filename, "rb");
  if (file==NULL)
    return i8086_ERR_FILENF;
  
  beginAdr = CAST_TO_MEMSIZE(beginAdr);
  r = fread(core->mem+beginAdr, sizeof(char), (i8086_MEM_SIZE*i8086_MAX_MEM_SEG)-beginAdr, file);

  if (r==0)
    return i8086_ERR_FILEREAD;
        
  fclose(file);
  
  if (beginAdr>0xffff)
  {
    cs = beginAdr / 0x10;
    i8086SetSegRegister(core, i8086_REG_CS, cs);
    core->pc = beginAdr % 0x10;
  }
  else
    core->pc = beginAdr % 0xffff;
  
  return i8086_SUC_FILELOAD;
}

/* Fuegt cmd in cmds an Position opcode ein */
/*
void i8086addCommand(i8086command **cmds, i8086command *cmd, unsigned char opcode)
{
  cmds[opcode] = cmd;
}
*/

/* Befehl in pc ausführen und vor Ausfuehrung           */
/* pc neusetzen.                                        */
/* cmds referenziert auf die Emulatorfunktionen         */
/* Return: Erfolg: i8086_SUC_EXECCMD                    */
/*         Illegaler Opcode: i8086_ERR_ILGOPCODE        */
int i8086execCommand(i8086core *core, i8086command **cmds)
{
  #define SYNC_CLOCKS 50000
  
  unsigned char opcode, i;
  unsigned char dispLn=0;
  i8086Parameter para, data;
  struct timezone tz;
  unsigned int cs;
  unsigned short trapflagbefore;
  signed char ret=i8086_SUC_EXECCMD;

  if (core->miscData.processedClocks == 0)
  {
    gettimeofday(&core->miscData.startTime, &tz);
  }
  
  cs = i8086GetSegRegister_fast(core, i8086_REG_CS, 1);
  opcode = core->mem[core->pc + cs];  /* opcode lesen */
  if (cmds[opcode]==NULL)
  {
    //i8086error(i8086_ERR_STR_ILGOPCODE,"");    
    return i8086_ERR_ILGOPCODE;
  }
    
  para.all = 0;
  for (i=1; i<cmds[opcode]->size; i++)            /* Parameter byteweise einlesen */
    para.b[i-1] = core->mem[/*CAST_TO_MEMSIZE*/(core->pc + cs + i)];

  if (cmds[opcode]->hasMod!=0)                    /* disp lesen */
  {
    dispLn=getAdditionalCmdLength(core, opcode, para.b[0]);
    for (i=0; i<dispLn; i++)
      para.b[i+cmds[opcode]->size-1] = core->mem[/*CAST_TO_MEMSIZE*/(core->pc + cs + i + cmds[opcode]->size)];
  }

  core->pc = core->pc + cmds[opcode]->size + dispLn; /* pc neu setzen */

  data.all = 0;

  trapflagbefore=core->flags & i8086_FLAG_TF;

  cmds[opcode]->func(core, opcode, para, data);      /* Emulatorfunktion aufrufen    */
  core->miscData.processedClocks += cmds[opcode]->clocks; /* Takte die 8086 benoetigen wurde speichern */

  if (!isPrefix(opcode))
  	core->miscData.segoverride = -1; /* segment-override beenden */
  else
    ret=i8086execCommand(core, cmds);    /* Opcode nach segment-override aufrufen */

  //Bei gesetztem Trapflag wird Int1 augerufen
  if ((core->flags & i8086_FLAG_TF) && trapflagbefore)
	{
		i8086callInterrupt(core,1);
	}


  if (core->miscData.processedClocks > SYNC_CLOCKS)       /* emu-Ausfuehrungszeit an 8086 anpassen. */
  {
    gettimeofday(&core->miscData.endTime, &tz);
    core->miscData.emuTime = core->miscData.endTime.tv_usec-core->miscData.startTime.tv_usec; /* Zeit die emu benoetigt speichern. */

    if ((core->miscData.endTime.tv_sec-1) <= (core->miscData.startTime.tv_sec)) /* Timerueberlaeufe abfangen */
      if (core->miscData.endTime.tv_usec < core->miscData.startTime.tv_usec)
        core->miscData.emuTime = core->miscData.endTime.tv_usec;
    if (core->miscData.emuTime > SYNC_CLOCKS*i8068_TPI)
      core->miscData.emuTime = SYNC_CLOCKS*i8068_TPI;

    cmdWait(core->miscData.processedClocks, core->miscData.emuTime);
    core->miscData.emuTime = 0;
    core->miscData.processedClocks = 0;
  }

  //if (core->miscData.segoverride == -1)
  i8086ProcessMsgQueue(core); /* Nachrichtenschlange abarbeiten */

  return ret;
}
