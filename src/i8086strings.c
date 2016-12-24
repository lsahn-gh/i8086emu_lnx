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
/* String-Manipulationsfunktionen		                */
/*                                                  */
/****************************************************/


#include "i8086proc.h"
//#include "i8086util.h"
#include "i8086emufuncs.h"
#include "i8086strings.h"


/*Autor: CST*/
/*Uebertraegt ein byte oder word von quelle ds:si nach ziel es:di und erhoeht si und di um 1bzw.2*/
void i8086Movs(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
    unsigned char wData=0, delta,byte;
    unsigned short word;
    //i8086SingleRegister *sireg,*direg;
    i8086SingleRegister sireg,direg;

    wData=opcode & i8086_BIT_0; //wData

    //sireg=i8086GetRegister(core, 1, i8086_REG_SI); /* SI lesen*/
    i8086GetRegisterRef(&sireg,core, 1, i8086_REG_SI); /* SI lesen*/
    //direg=i8086GetRegister(core, 1, i8086_REG_DI); /* DI lesen*/
    i8086GetRegisterRef(&direg,core, 1, i8086_REG_DI); /* DI lesen*/
    if(wData) //16 Bit Operation
    {
        word=memReadWord(core,sireg.x,i8086_REG_DS);//liest word aus ds:si
        memWriteWord(core,direg.x,word,i8086_REG_ES);//schreibt word nach es:di
        delta=2;//word
    }
    else//8 Bit Operation
    {
        byte=memReadByte(core,sireg.x,i8086_REG_DS);//liest byte aus ds:si
        memWriteByte(core,direg.x,byte,i8086_REG_ES);//schreibt byte nach es:di
        delta=1;//byte
    }
        if(core->flags & i8086_FLAG_DF) //directionflag gesetzt
        {
            sireg.x-=delta;//dekrementiere wert von SI um delta
            direg.x-=delta;//dekrementiere wert von DI um delta
        }
        else//directionflag nicht gesetzt
        {
            sireg.x+=delta;//inkrementiere wert von SI um delta
            direg.x+=delta;//inkrementiere wert von DI um delta
        }
      i8086SetRegister(core,i8086_REG_SI,1,sireg.x);//setzte SI
      i8086SetRegister(core,i8086_REG_DI,1,direg.x);//setze DI
}


/*Autor: CST*/
/*Uebertraegt ein byte oder word von quelle ds:si nach ziel AL/AX und erhoeht si um 1bzw.2*/
void i8086Lods(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
    unsigned char wData=0, delta,byte;
    unsigned short word;
    //i8086SingleRegister *sireg;
    i8086SingleRegister sireg;

    wData=opcode & i8086_BIT_0; //wData

        //sireg=i8086GetRegister(core, 1, i8086_REG_SI); /* SI lesen*/
	i8086GetRegisterRef(&sireg,core, 1, i8086_REG_SI); /* SI lesen*/
        if(wData) //16 Bit Operation
        {
            word=memReadWord(core,sireg.x,i8086_REG_DS);//liest word aus ds:si
            i8086SetRegister(core,i8086_REG_AX,wData,word);//schreibt das word nach AX
            delta=2;//word
        }
        else//8 Bit Operation
        {
            byte=memReadByte(core,sireg.x,i8086_REG_DS);//liest byte aus ds:si
            i8086SetRegister(core,i8086_REG_AL,wData,byte);//schreibt das byte nach AL
            delta=1;//byte
        }
        if(core->flags & i8086_FLAG_DF) //directionflag gesetzt
        {
            sireg.x-=delta;//dekrementiere wert von SI um delta
        }
        else//directionflag nicht gesetzt
        {
            sireg.x+=delta;//inkrementiere wert von SI um delta
        }
        i8086SetRegister(core,i8086_REG_SI,1,sireg.x);//setzte SI
}


/*Autor: CST*/
/*Uebertraegt ein byte oder word von quelle AL/AX nach ziel es:di und erhoeht di um 1bzw.2*/
void i8086Stos(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
    unsigned char wData=0, delta;
   //i8086SingleRegister *direg,*accreg;
    i8086SingleRegister direg,accreg;

    wData=opcode & i8086_BIT_0; //wData

    //direg=i8086GetRegister(core, 1, i8086_REG_DI); /* DI lesen*/
    i8086GetRegisterRef(&direg,core, 1, i8086_REG_DI); /* DI lesen*/
    //accreg=i8086GetRegister(core, wData, i8086_REG_AX); /* AX lesen*/
    i8086GetRegisterRef(&direg,core, wData, i8086_REG_AX); /* AX lesen*/
    if(wData) //16 Bit Operation
    {
        memWriteWord(core,direg.x,accreg.x,i8086_REG_ES);//schreibt word nach es:di
        delta=2;//word
    }
    else//8 Bit Operation
    {
        memWriteByte(core,direg.x,accreg.b[0],i8086_REG_ES);//schreibt byte nach es:di
        delta=1;//byte
    }
        if(core->flags & i8086_FLAG_DF) //directionflag gesetzt
        {
            direg.x-=delta;//dekrementiere wert von DI um delta
        }
        else//directionflag nicht gesetzt
        {
            direg.x+=delta;//inkrementiere wert von DI um delta
        }
        i8086SetRegister(core,i8086_REG_DI,1,direg.x);//setzte DI
}

/*Autor: CST **fehlt noch correct AF - wird von perrin in ovcaflagafteraddsub implementiert*/
/*Vergleicht byte oder word in es:di mit AL/AX und setzt die flags entsprechend - ergebnis wird nicht gespeichert!*/
void i8086Scas(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
    unsigned char wData=0, delta,ergb,byte;
    unsigned short word=0,ergw;
    //i8086SingleRegister *direg, *accreg;
    i8086SingleRegister direg, accreg;

    wData=opcode & i8086_BIT_0; //wData

    //direg=i8086GetRegister(core, 1, i8086_REG_DI); /* DI lesen*/
    i8086GetRegisterRef(&direg,core, 1, i8086_REG_DI); /* DI lesen*/
    //accreg=i8086GetRegister(core, wData, i8086_REG_AX); /* AX lesen*/
    i8086GetRegisterRef(&accreg,core, wData, i8086_REG_AX); /* AX lesen*/
    if(wData) //16 Bit Operation
    {
        word=memReadWord(core,direg.x,i8086_REG_ES);//word an es:di
        ergw=accreg.x-word;//subtrahiert word an es:di vom accreg
        correctArithmeticFlags(core,ergw,wData);//setzt ZF PF SF in abhaengigkeit vom ergebnis
        correctOvCaFlagafterAddSub(core,accreg.x,-word,ergw,wData,0);//setzt OF CF (AF) in abhaengigkeit vom ergebnis
        delta=2;//word
    }
    else//8 Bit Operation
    {
        byte=memReadByte(core,direg.x,i8086_REG_ES);//byte an es:di
        ergb=accreg.b[0]-byte;//subtrahiert byte an es:di vom accreg
        correctArithmeticFlags(core,ergb,wData);//setzt ZF PF SF in abhaengigkeit vom ergebnis
        correctOvCaFlagafterAddSub(core,accreg.b[0],-byte,ergb,wData,0);//setzt OF CF (AF) in abhaengigkeit vom ergebnis
        delta=1;//byte
    }
        if(core->flags & i8086_FLAG_DF) //directionflag gesetzt
        {
            direg.x-=delta;//dekrementiere wert von DI um delta
        }
        else//directionflag nicht gesetzt
        {
            direg.x+=delta;//inkrementiere wert von DI um delta
        }
        i8086SetRegister(core,i8086_REG_DI,1,direg.x);//setzte DI
}


/*Autor: CST */
/*Vergleicht byte oder word in es:di mit ds:si und setzt die flags entsprechend - ergebnis wird nicht gespeichert!*/
void i8086Cmps(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
    unsigned char wData=0, delta,ergb,bytesi=0,bytedi=0;
    unsigned short wordsi=0,worddi=0,ergw;
    i8086SingleRegister direg,sireg;

    wData=opcode & i8086_BIT_0; //wData

    //direg=i8086GetRegister(core, 1, i8086_REG_DI); /* DI lesen*/
    i8086GetRegisterRef(&direg,core, 1, i8086_REG_DI); /* DI lesen*/
    //sireg=i8086GetRegister(core, 1, i8086_REG_SI); /* SI lesen*/
    i8086GetRegisterRef(&sireg,core, 1, i8086_REG_SI); /* SI lesen*/
    if(wData) //16 Bit Operation
    {
        worddi=memReadWord(core,direg.x,i8086_REG_ES);//word an es:di
        wordsi=memReadWord(core,sireg.x,i8086_REG_DS);//word an ds:si
        ergw=wordsi-worddi;//subtrahiert word an es:di vom ds:si
        correctArithmeticFlags(core,ergw,wData);//setzt ZF PF SF in abhaengigkeit vom ergebnis
        correctOvCaFlagafterAddSub(core,wordsi,-worddi,ergw,wData,0);//setzt OF CF (AF) in abhaengigkeit vom ergebnis
        delta=2;//word
    }
    else//8 Bit Operation
    {
        bytedi=memReadByte(core,direg.x,i8086_REG_ES);//byte an es:di
        bytesi=memReadByte(core,sireg.x,i8086_REG_DS);//byte an ds:si
        ergb=bytesi-bytedi;//subtrahiert byte an es:di vom ds:si
        correctArithmeticFlags(core,ergb,wData);//setzt ZF PF SF in abhaengigkeit vom ergebnis
        correctOvCaFlagafterAddSub(core,bytesi,-bytedi,ergb,wData,0);//setzt OF CF (AF) in abhaengigkeit vom ergebnis
        delta=1;//byte
    }
        if(core->flags & i8086_FLAG_DF) //directionflag gesetzt
        {
            direg.x-=delta;//dekrementiere wert von DI um delta
            sireg.x-=delta;//dekrementiere wert von SI um delta
        }
        else//directionflag nicht gesetzt
        {
            direg.x+=delta;//inkrementiere wert von DI um delta
            sireg.x+=delta;//inkrementiere wert von SI um delta
        }
        i8086SetRegister(core,i8086_REG_DI,1,direg.x);//setzte DI
        i8086SetRegister(core,i8086_REG_SI,1,sireg.x);//setzte SI
   }

/*Autor: CST*/
/*Wiederholt den nachstehenden stringbefehl solange wie bzw. bis cx = 0 ist bzw. je nach rep nach dem ZF Flag*/
void i8086Rep(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SingleRegister cx;
  unsigned char repz;
  
  repz=opcode & i8086_BIT_0; //Repz oder Repnz
  i8086GetRegisterRef(&cx,core,1,i8086_REG_CX);//CX
                while(cx.x>0) //solange CX!=0
                {
                        i8086execCommand(core, commands);//string befehl ausführen
                        core->pc = core->pc -1;  // pc zurücksetzen (stringbefehle sind 1 byte gross!)
                        cx.x--;//cx dekrementieren
                        if(repz)//REPZ REPE REP
                        {
                                if(!(core->flags & i8086_FLAG_ZF)) break;//wenn ZF=0 ist abbrechen
                        }
                        else//REPNZ REPNE
                                if(core->flags & i8086_FLAG_ZF) break;//wenn ZF=1 ist abbrechen
                 }
  core->pc = core->pc +1;  //string befehl ueberspringen wenn kein rep
  i8086SetRegister(core,i8086_REG_CX,1,cx.x);//setzte CX
}
