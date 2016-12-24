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
/* Funktionen für den Datentransfer                 */
/*                                                  */
/****************************************************/

#include "i8086proc.h"
#include "i8086util.h"
#include "i8086emufuncs.h"
#include "i8086datatrans.h"
#include "i8086error.h"
#include "i8086messages.h"

#define MOV_DEST_REG_MASK 56
#define MOV_SOURCE_REG_MASK 7

#define MOV_GET_DEST_REG(para) (para & MOV_SOURCE_REG_MASK)
#define MOV_GET_SOURCE_REG(para) (para & MOV_DEST_REG_MASK)>>3

/*autor: jmh   */
/* mov reg,reg */
/* mov mem,reg */
/* mov reg,mem */
void i8086MovReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char destReg=0, sourceReg=0, wData=0;
  signed short disp;
  i808616BitAdr ea;
  i8086SingleRegister sreg;

  wData = opcode & i8086_BIT_0;                      /* 1=wReg, 0=bReg           */
  if ((para.b[0] & i8086_BIT_6) && (para.b[0] & i8086_BIT_7))       /* Reg to Reg */
  {
    if(opcode & i8086_BIT_1) //Reg TO Reg
    {
       sourceReg = getBitSnipped(para.b[0], 2, 3);/* Register-Code für Source   */
       destReg = getBitSnipped(para.b[0], 5, 3);/* Register-Code für Dest */
    }
    else //Reg FROM Reg
    {
        destReg = getBitSnipped(para.b[0], 2, 3);/* Register-Code für Dest   */
        sourceReg = getBitSnipped(para.b[0], 5, 3);/* Register-Code für Source */
    }
    i8086GetRegisterRef(&sreg, core, wData, sourceReg);/* Source-Register lesen    */
    i8086SetRegister(core, destReg, wData, sreg.x); /* Dest-Register schreiben  */
  }
  else
  {
    disp=joinBytes(para.b[1],para.b[2]);//komischerweise steht hier die sourceadr drin
    ea=decodeMemAdr(core, para.b[0], disp);//berechne adresse
    if(opcode & i8086_BIT_1) //mov mem TO reg
    {
      destReg = getBitSnipped(para.b[0], 5, 3);//destregister
      if(wData) //16 Bit Operation
         i8086SetRegister(core, destReg, wData, memReadWord(core,ea,i8086_REG_DS)); /* Destregister schreiben  */
      else //8 Bit Operation
         i8086SetRegister(core, destReg, wData, memReadByte(core,ea,i8086_REG_DS)); /* Destregister schreiben  */
    }
    else //mov mem FROM reg
    {
      sourceReg = getBitSnipped(para.b[0], 5, 3);//sourceregister
      i8086GetRegisterRef(&sreg,core, wData, sourceReg);
      if(wData) //16 Bit Operation
         memWriteWord(core,ea,sreg.x,i8086_REG_DS);//schreibe inhalt von sourceregister in speicher
      else //8 Bit Operation
         memWriteByte(core,ea,sreg.b[0],i8086_REG_DS);//schreibe inhalt von sourceregister in speicher
    }
  }
}

/*autor: cst*/
/*mov acc,mem16/8 mov mem16/8,acc*/
void i8086MovAcc(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char wData=0;
  i808616BitAdr ea;
  i8086SingleRegister sreg;

  wData=opcode & i8086_BIT_0;
  ea=joinBytes(para.b[1],para.b[0]);//EA setzt sich aus den 1. beiden parameterbytes zusammen!
  i8086GetRegisterRef(&sreg,core,wData,i8086_REG_AX); //register ax bzw. al = acc
  if(opcode & i8086_BIT_1) //mov ACC to MEM
  {
    if(wData) //16 Bit Operation
    {
      memWriteWord(core,ea,sreg.x,i8086_REG_DS); //schreibe inhalt von AX in den Speicher an die Adr. EA
    }
    else //8 Bit Operation
    {
      memWriteByte(core,ea,sreg.b[0],i8086_REG_DS); //schreibe inhalt von AL in den Speicher an die Adr EA
    }
  }
  else //mov MEM to ACC
  {
    if(wData) //16 Bit Operation
    {
      i8086SetRegister(core,i8086_REG_AX,wData,memReadWord(core,ea,i8086_REG_DS)); //schreibe Word an der Adresse EA in AX
    }
    else //8 Bit Operation
    {
      i8086SetRegister(core,i8086_REG_AL,wData,memReadByte(core,ea,i8086_REG_DS)); //schreibe Byte an der Adresse EA in AL
    }
  }
}

/*autor: cst*/
/*mov segreg,reg16/8/mem mov reg16/8/mem,segreg */
void i8086MovSeg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char segReg=0,reg=0,wData=1; //segmentregister sind immer 16 bit!
  signed short disp;
  unsigned short segregdata;
  i808616BitAdr ea;
  i8086SingleRegister regptr;

  disp=joinBytes(para.b[1],para.b[2]);//DISPLACEMENT
  segReg=getBitSnipped(para.b[0], 4, 2);//destsegmentregister
  if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //mov from bzw. to REG
  {
    reg=getBitSnipped(para.b[0], 2, 3);//sourceregister

    if(opcode & (i8086_BIT_1)) //mov REG to SEGREG
    {
      i8086GetRegisterRef(&regptr,core,wData,reg); //sourceregister
      i8086SetSegRegister(core, segReg, regptr.x);//speichere inhalt von reg in das segreg
    }
    else//mov SEGREG to REG
    {
      segregdata=i8086GetSegRegister_fast(core, segReg, 0);//destsegmentregister
      i8086SetRegister(core,reg,wData,segregdata);//speichere inhalt von segreg in das reg
    }
  }
  else //mov from bzw. to MEM
  {
    ea=decodeMemAdr(core, para.b[0], disp);//berechne adresse

    if(opcode & (i8086_BIT_1)) //mov MEM to SEGREG
    {
      i8086SetSegRegister(core, segReg, memReadWord(core,ea,i8086_REG_DS));//speichere inhalt von MEM an der Adr EA in das segreg
    }
    else//mov SEGREG to MEM
    {
      segregdata = i8086GetSegRegister_fast(core, segReg, 0);//destsegmentregister
      memWriteWord(core,ea,segregdata,i8086_REG_DS);//speichere inhalt vom segmentregister in MEM ander Adr EA
    }
  }
}


/*autor: cst*/
/*mov reg16/8,const mov mem8/16,const */
void i8086MovConst(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char destReg=0,wData=0,al=0;
  signed short disp, immed; //immed = konstante
  i808616BitAdr ea;

  al=getAdditionalCmdLength(core, opcode, para.b[0]); //Länge von Disp
  if(opcode & i8086_BIT_5) //mov const to reg8/16
  {
    wData=opcode & i8086_BIT_3;//16Bit oder 8Bit?
    destReg=getBitSnipped(opcode, 2, 3);//destinationregister
    if(wData) //16 Bit Operation
    {
      immed=joinBytes(para.b[1],para.b[0]);//immed sind die 1. zwei parameterbytes
      i8086SetRegister(core,destReg,wData,immed);//speichere konstante in destination register
    }
    else //8 Bit Operation
    {
      immed=para.b[0]; //immed ist nur das erste parameterbyte
      i8086SetRegister(core,destReg,wData,immed);//speichere konstante in destination register
    }
  }
  else//mov const to mem8/16
  {
    wData=opcode & i8086_BIT_0;
    disp=joinBytes(para.b[1],para.b[2]);
    ea=decodeMemAdr(core, para.b[0], disp); //adresse

    if(wData) //16 Bit Operation hier wird das 0. parameter byte ignoriert!!! WARNING! - was meinst du???
    {
      immed=joinBytes(para.b[al+2],para.b[al+1]); //immed 4.+5. byte
      memWriteWord(core,ea,immed,i8086_REG_DS);//schreibe konstante in speicher an der adr EA
    }
    else //8 Bit Operation
    {
      immed=para.b[al+1]; //immed 4. byte
      memWriteByte(core,ea,immed,i8086_REG_DS);//schreibe konstante in speicher an der adr EA
    }
  }
}

/*autor: cst*/
/* xchg mem,reg  */
/* xchg reg,mem  */
/* xchg reg,reg  */
/* xchg acc,reg */
/* xchg reg,acc */
void i8086XchgReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char destReg=0, sourceReg=0, wData=0, templo;
  signed short disp,temp;/*variablen fuer xchg mem,reg bzw. xchg reg,mem */
  i808616BitAdr ea;
  i8086SingleRegister sreg;

  wData = opcode & i8086_BIT_0;                    /* 1=wReg, 0=bReg           */
  if ( ((para.b[0] & i8086_BIT_6) && (para.b[0] & i8086_BIT_7)) || para.b[0]==0)       /* Reg to Reg */
  {
    if(para.b[0]!=0)      /*hat der Befehl nur 1 byte groesse?*/
    {
      sourceReg = getBitSnipped(para.b[0], 5, 3);  /*normaler xchg reg,reg*/
      destReg = getBitSnipped(para.b[0], 2, 3);
    }
    else
    {
       sourceReg = getBitSnipped(opcode, 2, 3);           /* Xchg acc, reg bzw. reg,acc   */
       destReg = i8086_REG_AX;
       wData=1; //immer 16 Bit Operation!
    }
    
    i8086GetRegisterRef(&sreg,core, wData, sourceReg);

    temp=sreg.x;                                            /* Inhalt vom Source-Register zwischenspeichern    */
    
    i8086GetRegisterRef(&sreg,core, wData, destReg);           /*Inhalt vom Dest-Register lesen*/

    i8086SetRegister(core, sourceReg, wData, sreg.x); /* Source-Register schreiben  */
    i8086SetRegister(core, destReg, wData, temp); /* Dest-Register schreiben  */

  }
  else    /* XCHG REG, MEM || XCHG MEM,REG*/
  {
      disp=joinBytes(para.b[1],para.b[2]); //hier steht komischerweise die mem16 adresse
      ea=decodeMemAdr(core, para.b[0], disp); //adresse mit der vertauscht wird
      sourceReg=getBitSnipped(para.b[0], 5, 3); //das register mit dem vertauscht wird
      i8086GetRegisterRef(&sreg,core, wData, sourceReg); //register
      temp=sreg.x;//speichere inhalt des Registers zwischen
      if(wData) //ist es eine 16bit operation?
      {
        i8086SetRegister(core, sourceReg, wData, memReadWord(core,ea,i8086_REG_DS));//16bit des registers mit speicherinhalt
        memWriteWord(core,ea,temp,i8086_REG_DS);//fuelle speicher an adr.
      }
      else //8 Bit Operation
      {
        templo=sreg.b[0];
        i8086SetRegister(core, sourceReg, wData, memReadByte(core,ea,i8086_REG_DS));//8bit des registers mit speicherinhalt
        memWriteByte(core,ea,templo,i8086_REG_DS);//fuelle speicher an adr.
      }
  }
}

/*autor: cst*/
/* movs flags to AH */
void i8086Lahf(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SetRegister(core, i8086_REG_AH, 0, core->flags); //mov SF:ZF:xx:AF:xx:PF:xx:CF to ah xx-unbestimmt
}
/*autor: cst*/
/* set flags from AH */
void i8086Sahf(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SingleRegister sreg;

  i8086GetRegisterRef(&sreg,core, 0, i8086_REG_AH); /* AH-Register lesen    */
  /*OPTIMIERUNG*/
   core->flags=(core->flags & 0xff00)|sreg.b[0];//maske 1111111100000000 anlegen und die unteren 8bit durch AH ersetzen
}

/*autor cst */
/* bestmögliche beschreibung dieses befehls MOV AL,[BX+AL]*/
void i8086Xlat(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned short offset;
  i8086SingleRegister regbx,regal;

  i8086GetRegisterRef(&regal,core, 0, i8086_REG_AL); //reg al
  i8086GetRegisterRef(&regbx,core, 1, i8086_REG_BX); //reg bx
  offset=regbx.x+regal.b[0];  //berechnung der speicheradresse aus al und bx
  i8086SetRegister(core, i8086_REG_AL, 0, memReadByte(core,offset,i8086_REG_DS));//schreibe den wert an der speicheradresse offset nach al
}

/*autor cst*/
/* PUSH REG bzw. PUSH SEGMENTREG */
void i8086PushReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char sourceReg;
  unsigned short segregdata;
  i8086SingleRegister sreg, stackreg;

  i8086GetRegisterRef(&stackreg,core, 1, i8086_REG_SP); //stackregister
  stackreg.x-=2;//dekrementiere stackpointer um 2
  i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x);//schreibe stackpointer ins stackregister

  if(opcode & i8086_BIT_6) //push reg
  {
    sourceReg=getBitSnipped(opcode, 2, 3); //bit 0 1 und 2 des opcodes = sourceReg
    i8086GetRegisterRef(&sreg,core, 1, sourceReg); //Source Reg
    memWriteWord(core, stackreg.x, sreg.x,i8086_REG_SS);// schreibe inhalt vom Source Reg in den Speicher an der Stelle vom Stackpointer
  }
  else //push segment reg
  {
    sourceReg=getBitSnipped(opcode, 4, 2); //bit 3 und 4 des opcodes = sourceSegReg
    segregdata=i8086GetSegRegister_fast(core, sourceReg, 0); //Source SegReg
    memWriteWord(core, stackreg.x, segregdata,i8086_REG_SS);// schreibe inhalt vom Source Reg in den Speicher an der Stelle vom Stackpointer
  }
}

/*autor cst*/
/* PUSH MEM */
void i8086PushMem(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

  i808616BitAdr sourceAdr;
  i8086SingleRegister stackreg;
  unsigned short disp;

  disp=joinBytes(para.b[1],para.b[2]);//DISPLACEMENT

  i8086GetRegisterRef(&stackreg,core, 1, i8086_REG_SP);//stackregister
  stackreg.x-=2;//dekrementiere stackpointer 2
  i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x);//schreibe stackpointer in stackregister

  sourceAdr=decodeMemAdr(core, para.b[0], disp);//berechnet adresse
  memWriteWord(core,stackreg.x,memReadWord(core,sourceAdr,i8086_REG_DS),i8086_REG_SS);// schreibe inhalt vom Source Mem in den Speicher an der Stelle vom Stackpointer
}


/*autor cst*/
/* POP REG bzw. POP SEGMENTREG */

void i8086PopReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

  unsigned char destReg;
  i8086SingleRegister stackreg;


  i8086GetRegisterRef(&stackreg,core, 1, i8086_REG_SP); //stackregister mit stackpointer

  if(opcode & i8086_BIT_6) //pop reg
  {
    destReg=getBitSnipped(opcode, 2, 3); //bit 0 1 und 2 des opcodes = destReg
     
    i8086SetRegister(core, destReg, 1, memReadWord(core, stackreg.x,i8086_REG_SS));//schreibe inhalt vom Speicher an der Stelle vom Stackpointer, in das destination reg
  }
  else //pop segment reg
  {
    destReg=getBitSnipped(opcode, 4, 2); //bit 3 und 4 des opcodes = destReg
    i8086SetSegRegister(core, destReg, memReadWord(core, stackreg.x,i8086_REG_SS));//schreibe inhalt vom Speicher an der Stelle vom Stackpointer, in das destination segreg
  }


   i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x+2);//setze neuen stackpointer in stackregister
}


/*autor cst*/
/* POP MEM */
void i8086PopMem(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

  i808616BitAdr sourceAdr;
  i8086SingleRegister stackreg;
  unsigned short disp;

  disp=joinBytes(para.b[1],para.b[2]);

  i8086GetRegisterRef(&stackreg,core, 1, i8086_REG_SP);

  sourceAdr=decodeMemAdr(core, para.b[0], disp); //source adresse
  memWriteWord(core,sourceAdr,memReadWord(core,stackreg.x,i8086_REG_SS),i8086_REG_DS);//schreibe oberen stapelinhalt in destination mem

  stackreg.x+=2;//inkrementiere stackpointer um 2
  i8086SetRegister(core, i8086_REG_SP, 1, stackreg.x);//speichere stackpointer ins stackregister
}


/*autor cst ** */
/* LEA reg,mem-> speichert das offset der effective adress (EA) im REGISTER REG */
/*beispiel lea ax, [bx+si+10h] speicher die summe bx+si+10h in ax*/
void i8086Lea(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i808616BitAdr ea;
  unsigned short disp, wData=1, destReg=0;

  disp=joinBytes(para.b[1],para.b[2]);//DISPLACEMENT
  ea=decodeMemAdr(core,para.b[0], disp);//Adresse

  destReg=getBitSnipped(para.b[0], 5, 3);//destination register
  i8086SetRegister(core, destReg,wData,ea);//schreibe die berechnete adresse ins dest reg
}

/*autor cst ** */
/* LDS/LES - lesen von der source adresse 32bit die ersten 16bit werden als offset ins zielregister geschrieben */
/*die zweiten 16 bit als segment nach ds bzw. es*/
/*lds les reg16/mem16*/
void i8086LdsLes(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i808616BitAdr ea;
  unsigned short disp, destReg=0, segReg=0,words;
  
  if(opcode & i8086_BIT_0) // 0. bit 1?
        segReg=i8086_REG_DS; //dann LDS
  else
        segReg=i8086_REG_ES;//sonst LES

  disp=joinBytes(para.b[1],para.b[2]);//DISPLACMENT
  ea=decodeMemAdr(core,para.b[0], disp);//Adresse
  destReg=getBitSnipped(para.b[0], 5, 3);//destination register
  words=memReadWord(core,ea,i8086_REG_DS);//lese die ersten 16bit  an der adresse aus dem speicher
  i8086SetRegister(core, destReg,1,words);//schreibe diese 16 bit ins dest reg
  words=memReadWord(core,ea+2,i8086_REG_DS);//lese die zweiten 16bit an der adresse aus dem speicher
  i8086SetSegRegister(core, segReg,words);//schreibe diese 16bit als segment ins DS Register
}




/*autor cst*/
/* PUSHF - PUSH Flags auf den Stack */
void i8086Pushf(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned short wData=1;
  i8086SingleRegister stackreg;

  i8086GetRegisterRef(&stackreg,core,wData,i8086_REG_SP);
  stackreg.x-=2;
  i8086SetRegister(core,i8086_REG_SP,wData,stackreg.x);
  memWriteWord(core,stackreg.x,core->flags,i8086_REG_SS);
}

/*autor cst*/
/* POPF - POP Flags vom Stack */
void i8086Popf(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned short wData=1;
  i8086SingleRegister stackreg;

  i8086GetRegisterRef(&stackreg,core,wData,i8086_REG_SP);
  core->flags=memReadWord(core,stackreg.x,i8086_REG_SS) | i8086_CPU_SIG; //obere 4bits immer 1
  stackreg.x+=2;
  i8086SetRegister(core,i8086_REG_SP,wData,stackreg.x);
}

/*autor cst*/
/* IN al, port */
/* IN al, DX */
/* IN ax, port */
/* IN ax, DX */
void i8086In(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned short acc=i8086_REG_AX,dx=i8086_REG_DX,wData=0;
  i8086SingleRegister dxreg;
  unsigned short portNum=0;

  wData=opcode & i8086_BIT_0; //16 Bit Flag

  if(opcode & i8086_BIT_3) //IN acc, PORT -> PORT steht in DX
  {
    i8086GetRegisterRef(&dxreg,core,1,dx);//dx register hier steht der port drin!
    if(dxreg.x>=i8086_PORTS) //illegaler port?
      i8086error("Zugriffsverletzung Illegaler I/O Port!" , "");//error
    if(wData) //16 Bit Operation
    {
      i8086SetRegister(core,acc,wData,portOpWord(core,i8086_INPUT_PORT,0,dxreg.x,i8086_READ_PORT));////schreibe word aus port in dx nach acc
    }
    else //8Bit Operation
    {
      i8086SetRegister(core,acc,wData,portOpByte(core,i8086_INPUT_PORT,0,dxreg.x,i8086_READ_PORT));//schreibe byte aus port in dx nach acc
    }
    portNum = dxreg.x;
  }
  else //IN acc, PORT=CONST (CONST only 0...255)
  {
     if(wData) //16 Bit Operation
    {
      i8086SetRegister(core,acc,wData,portOpWord(core,i8086_INPUT_PORT,0,para.b[0],i8086_READ_PORT));//schreibe word aus port nach acc
    }
    else //8Bit Operation
    {
      i8086SetRegister(core,acc,wData,portOpByte(core,i8086_INPUT_PORT,0,para.b[0],i8086_READ_PORT));//schreibe byte aus port nach acc
    }
    portNum = para.b[0];
  }
  //i8086PushMsg(i8086_SIG_PORT_IN, portNum, 0); /* signalisiere INPORT Operation */
}

/*autor cst*/
/* OUT port, al */
/* OUT port, ax */
/* OUT dx, al */
/* OUT dx, ax*/
void i8086Out(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned short acc=i8086_REG_AX,dx=i8086_REG_DX,wData=0;
  short error;
  i8086SingleRegister dxreg,accreg;
  unsigned short portNum=0;
  int value=0;

  wData=opcode & i8086_BIT_0; //16 Bit Flag

  i8086GetRegisterRef(&accreg,core,wData,acc);//accu register (al oder ax)
  if(opcode & i8086_BIT_3) //IN PORT, acc -> PORT steht in DX
  {
    i8086GetRegisterRef(&dxreg,core,1,dx);//dx register hier steht der port drin!
    if(wData) //16 Bit Operation
    {
      error=portOpWord(core,i8086_OUTPUT_PORT,accreg.x,dxreg.x,i8086_WRITE_PORT);//schreibe das word aus acc in port aus dx
    }
    else //8Bit Operation
    {
      error=portOpByte(core,i8086_OUTPUT_PORT,accreg.b[0],dxreg.x,i8086_WRITE_PORT);//schreibe das byte aus acc in port aus dx
    }
    portNum = dxreg.x;
  }
  else //IN PORT=CONST, acc (CONST only 0...255)
  {
    if(wData) //16 Bit Operation
    {
      error=portOpWord(core,i8086_OUTPUT_PORT,accreg.x,para.b[0],i8086_WRITE_PORT);//schreibe word aus acc in den port
    }
    else //8Bit Operation
    {
      error=portOpByte(core,i8086_OUTPUT_PORT,accreg.x,para.b[0],i8086_WRITE_PORT);//schreibe byte aus acc in den port
    }
    portNum = para.b[0];
  }
  
  value = accreg.x;
  if(error==i8086_ERR_ILGPORT)
    i8086error("Zugriffsverletzung Illegaler I/O Port!" , ""); /* error */
  i8086PushMsg(i8086_SIG_PORT_OUT, portNum, value); /* signalisiere OUTPORT Operation */
}
