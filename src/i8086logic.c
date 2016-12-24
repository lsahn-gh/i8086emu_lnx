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
/* Autor: CST/FB/RD                                 */
/* Version: 1.0                                     */
/*                                                  */
/* Logikfunktionen                                  */
/*                                                  */
/****************************************************/

#include "i8086proc.h"
#include "i8086util.h"
#include "i8086emufuncs.h"

/*MSB - Operationen*/
#define GET_MSB(data,wData) wData?(data&32768):(data&i8086_BIT_7)
#define GET_LSB(data) data & 1
#define SET_MSB(data,wData) wData?(data|=(1<<15)):(data|=(1<<7))


//Autor: FB 
/* Not  */
void i8086not(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  unsigned char wData=0;
  i808616BitAdr adr;
  i8086SingleRegister sreg;


  wData = opcode & i8086_BIT_0;                     /* 1=wReg, 0=bReg           */

  if ((para.b[0] & 192) == 192) //Reg - bit 6 und bit7 von para.b[0] sind 1
  {
    i8086GetRegisterRef(&sreg,core, wData, para.b[0]&7);    /* Source-Register lesen    */
    i8086SetRegister(core, para.b[0]&7, wData,~(sreg.x)); /* Dest-Register schreiben  */
  }
  else  //not MEM
    {
      adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));  //adr berechnen
      //16bit
      if( opcode & 1)
        {
        memWriteWord(core,adr,~(memReadWord(core,adr,i8086_REG_DS)),i8086_REG_DS);  //16bitadr
        }
      //8bit
      else
        memWriteByte(core,adr,~(memReadByte(core,adr,i8086_REG_DS)),i8086_REG_DS);  //8bit adr
    }
}

/*Autor:CST*/
/*SHL REG, SHL MEM*/
/*SHR REG, SHR MEM*/
/*SAR REG, SAR MEM*/
/* Shift left - Shift Right - Shift arithmetic right */
void i8086ShlShrSar(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister sreg,clreg;
	i808616BitAdr ea;
	unsigned short disp,oldmsb,msb,shldata;
	unsigned char wData,vData,destReg,shifts=1,shr=0,sar=0;
	
	wData=opcode & i8086_BIT_0;//16 oder 8 Bit Operation
	vData=opcode & i8086_BIT_1;//SHL/SHR um 1 oder um n bits
	shr=para.b[0] & i8086_BIT_3; //soll ein SHR ausgefuehrt werden?
	sar=para.b[0] & i8086_BIT_4; //soll ein SAR ausgefuehrt werden?
	if(vData) //mehr als 1 shift
	{
	i8086GetRegisterRef(&clreg,core,0,i8086_REG_CL); //CL Register
		shifts=clreg.b[0]; //anzahl der shifts
	}

	if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //SHL/SHR REG
	{
		destReg=getBitSnipped(para.b[0], 2, 3);//nummer des Registers
		i8086GetRegisterRef(&sreg,core,wData,destReg); //destination Register
		if(wData)//16 Bit Operation
			shldata=sreg.x;//zu shiftende daten	
		else
			shldata=sreg.b[0];
	}
	else//SHL/SHR MEM
	{
		disp=joinBytes(para.b[1],para.b[2]);
		ea=decodeMemAdr(core, para.b[0],disp);  //Adresse berechnen
		shldata=memRead(core,ea,wData,i8086_REG_DS);//zu shiftende daten
	}
			oldmsb=GET_MSB(shldata,wData);//nur für 1bit shift notwenidg
			if(shifts) //shifts > 0
                        {
				if(shr)//SHR oder SAR
                                {
					if(sar)
						shldata=((signed short)shldata)>>(shifts-1);//bis shifts-1 arithmetisch shiften
					else
						shldata=shldata>>(shifts-1);//bis shifts-1 shiften
                                }
				else//SHL
					shldata=shldata<<(shifts-1);//bis shifts-1 shiften
                        }
		if((shr) || (sar))//SHR oder SAR
			if(GET_LSB(shldata))//speichert das rausgeshiftete LSB nach CF
				core->flags|=i8086_FLAG_CF; 
			else
				core->flags&=~(i8086_FLAG_CF);
		else
                        if((msb=GET_MSB(shldata,wData)))//speichert das rausgeshiftete MSB nach CF
				core->flags|=i8086_FLAG_CF; 
			else
				core->flags&=~i8086_FLAG_CF;
			if(shifts) //shifts >0
                        {
				if(shr)//SHR oder SAR
                                {
					if(sar)
						shldata=((signed short)shldata)>>1;//arithmetischen shift beenden
					else
						shldata=shldata>>1;//shift beenden
                               }
				else
					shldata=shldata<<1;//shift beenden
                        }
	if(shifts==1) //wenn sich bei 1bit shifts das MSB geändert hat wirds OF gesetzt
        {
		if(sar) //SAR
			core->flags&=~i8086_FLAG_OF;//OF loeschen bei SAR um 1 bit
		else if(oldmsb != (msb=GET_MSB(shldata,wData)))//hat sich das vorzeichen geaendert?
			core->flags|=i8086_FLAG_OF; //OF setzen
		else
			core->flags&=~i8086_FLAG_OF; //OF loeschen
        }
	correctArithmeticFlags(core, shldata,wData);//korrigiert ZF SF PF
	if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //SHL/SHR REG
		i8086SetRegister(core, destReg, wData,shldata); /* Dest-Register schreiben  */
	else  //SHL/SHR MEM
		memWrite(core, ea, shldata, wData, i8086_REG_DS);/*DEST-Mem schreiben*/
}

/*Autor:CST*/
/*ROL REG, ROL MEM*/
/*ROR REG, ROR MEM*/
/*RCL REG, RCL MEM*/
/*RCR REG, RCR MEM*/
/* Rotate left - Rotate Right*/
/* Rotate through carry left - Rotate through carry Right*/
void i8086RolRorRclRcr(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
        i8086SingleRegister sreg,clreg;
        i808616BitAdr ea;
        unsigned short disp,oldmsb,msb,lsb,rordata;
        unsigned char wData,vData,destReg,shifts=1,i,ror,withcarry;
        
        wData=opcode & i8086_BIT_0;//16 oder 8 Bit Operation
        vData=opcode & i8086_BIT_1;//ROR/ROL um 1 oder um n bits
        ror=para.b[0] & i8086_BIT_3;//operation nach rechts
        withcarry=para.b[0] & i8086_BIT_4; //rotate with carry

        if(vData) //mehr als 1 shift
	{
	        i8086GetRegisterRef(&clreg,core,0,i8086_REG_CL); //CL Register
		shifts=clreg.b[0]; //anzahl der shifts
	}
        i=shifts;//zaehler initialisieren;
        if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //ROL/ROR REG
	{
		destReg=getBitSnipped(para.b[0], 2, 3);//nummer des Registers
		i8086GetRegisterRef(&sreg,core,wData,destReg); //destination Register
		if(wData)//16 Bit Operation
			rordata=sreg.x;//zu shiftende daten	
		else
			rordata=sreg.b[0];
	}
	else//ROL/ROR MEM
	{
		disp=joinBytes(para.b[1],para.b[2]);
		ea=decodeMemAdr(core, para.b[0],disp);  //Adresse berechnen
		rordata=memRead(core,ea,wData,i8086_REG_DS);//zu shiftende daten
	}
        oldmsb=GET_MSB(rordata,wData);//nur für 1bit shift notwenidg
			while(i-1) //i-1 > 0
                        {        
				if(ror)//ROR
                                {       
                                        lsb=GET_LSB(rordata); 
                                        rordata=rordata>>1;//um 1 bit shiften
                                                if(withcarry) //rotate with carry
                                                {
                                                        if(core->flags & i8086_FLAG_CF) SET_MSB(rordata,wData); //wenn CF gesetzt war wird MSB gesetzt
                                                        if(lsb) core->flags|=i8086_FLAG_CF; //wenn lsb gesetzt war wird CF gesetzt
                                                        else   core->flags&=~i8086_FLAG_CF;//wenn lsb nicht gesetzt war wird CF geloescht
                                                }
                                                else//rotate without carry
                                                        if(lsb)  SET_MSB(rordata,wData); //wenn lsb gesetzt war wird MSB gesetzt
                                 }
				else//ROL
                                {
                                        msb=GET_MSB(rordata,wData);
					rordata=rordata<<1;//um 1 bit shiften
                                                if(withcarry) //rotate with carry
                                                {
                                                        if(core->flags & i8086_FLAG_CF) rordata+=1; //wenn CF gesetzt war wird LSB gesetzt
                                                        if(msb) core->flags|=i8086_FLAG_CF; //wenn msb gesetzt war wird CF gesetzt
                                                        else   core->flags&=~i8086_FLAG_CF;//wenn msb nicht gesetzt war wird CF geloescht
                                                }
                                                else//rotate without carry
                                                        if(msb) rordata+=1; //wenn MSB gesetzt war wird das LSB gesetzt
                                }
                                        
                          i--;//i dekrementieren
                         }
		if(ror && !withcarry)//ROR aber nicht mit carry
			if((lsb=GET_LSB(rordata)))//speichert das rausgeshiftete LSB nach CF
				core->flags|=i8086_FLAG_CF; 
			else
				core->flags&=~i8086_FLAG_CF;
		else//ROL
                        if((msb=GET_MSB(rordata,wData)))//speichert das rausgeshiftete MSB nach CF
				core->flags|=i8086_FLAG_CF; 
			else
				core->flags&=~i8086_FLAG_CF;
			if(shifts) //shifts >0
                        {
				if(ror)//ROR
                                {
                                        lsb=GET_LSB(rordata);//LSB speichern
                                        rordata=rordata>>1;//shift beenden
                                                 if(withcarry) //rotate with carry
                                                {
                                                        if(core->flags & i8086_FLAG_CF) SET_MSB(rordata,wData); //wenn CF gesetzt war wird MSB gesetzt
                                                        if(lsb) core->flags|=i8086_FLAG_CF; //wenn lsb gesetzt war wird CF gesetzt
                                                        else   core->flags&=~i8086_FLAG_CF;//wenn lsb nicht gesetzt war wird CF geloescht
                                                }
                                                else//rotate without carry
                                                        if(lsb) SET_MSB(rordata,wData);//wenn LSB gesetzt war wird MSB gesetzt
                                }
				else
                                {
                                        msb=GET_MSB(rordata,wData);
					rordata=rordata<<1;//shift beenden
                                                 if(withcarry) //rotate with carry
                                                {
                                                        if(core->flags & i8086_FLAG_CF) rordata+=1; //wenn CF gesetzt war wird LSB gesetzt
                                                        if(msb) core->flags|=i8086_FLAG_CF; //wenn msb gesetzt war wird CF gesetzt
                                                        else   core->flags&=~i8086_FLAG_CF;//wenn msb nicht gesetzt war wird CF geloescht
                                                }
                                                else//rotate without carry
                                                        if(msb) rordata+=1;//wenn MSB gesetzt war wird LSB gesetzt
                                }
                           }
	if(shifts==1) //wenn sich bei 1bit shifts das MSB geändert hat wirds OF gesetzt
        {
                if(oldmsb != (msb=GET_MSB(rordata,wData)))//hat sich das vorzeichen geaendert?
			core->flags|=i8086_FLAG_OF; //OF setzen
		else
			core->flags&=~i8086_FLAG_OF; //OF loeschen
        }
	if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //ROR/ROL REG
		i8086SetRegister(core, destReg, wData,rordata); /* Dest-Register schreiben  */
	else  //ROL/ROR MEM
		memWrite(core, ea, rordata, wData, i8086_REG_DS);/*DEST-Mem schreiben*/
}

/*Autor:CST*/
/*And Immed to Mem/Reg*/
/*Or Immed to Mem/Reg*/
/*Xor Immed to Mem/Reg*/
/*Test Immed to Mem/Reg ** es wird die falsche laenge berechnet!*/
void i8086AndTestOrXorImmed(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SingleRegister dreg;
  unsigned short result,immed,sdata;
  unsigned char wData,destReg,op,al;
  i808616BitAdr ea;
  
  wData=opcode & i8086_BIT_0;//16bit oder 8 bit operation
  op=getBitSnipped(para.b[0],5,3); //auslesen der art der operation (3-5.bit des ersten parameters)
  al=getAdditionalCmdLength(core, opcode, para.b[0]);
  
          if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //Operation mit REG
          {
                  destReg=getBitSnipped(para.b[0],2,3); //Destination Register
                  i8086GetRegisterRef(&dreg,core,wData,destReg);//Register holen
                  sdata=dreg.x;//operand
                  if(wData)//16Bit Operation
                          immed=joinBytes(para.b[2],para.b[1]);//immed - 2. operand
                  else//8Bit Operation
                          immed=para.b[1];//immed - 2. operand
          }
          else//Operation mit MEM
          {
                  ea=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));//Adresse
                  sdata=memRead(core,ea,wData,i8086_REG_DS);//operand
                  if(wData)//16Bit Operation
                          immed=joinBytes(para.b[al+2],para.b[al+1]);//immed - 2. operand
                  else//8Bit Operation
                          immed=para.b[al+1];//immed - 2. operand
          }
          
          
          core->flags&=~i8086_FLAG_CF;  //Carry Flag 0 setzen
          core->flags&=~i8086_FLAG_OF;  //Overflow Flag 0 setzen
                  if(op==1) //OR
                          result=immed | sdata;//OR
                  else if(op==6)//XOR
                          result=immed ^ sdata; //XOR
                  else
                          result=immed & sdata;//AND bzw. TEST
           correctArithmeticFlags(core, result,wData);//korrigiert ZF PF SF anhand des ergebnisses
           if(op!=0)//wenn es sich NICHT um Test handelt
           {
                   if((para.b[0] & i8086_BIT_7) && (para.b[0] & i8086_BIT_6)) //Operation mit REG
                        i8086SetRegister(core, destReg, wData,result); /* Dest-Register schreiben  */
                   else  //Operation mit MEM
                        memWrite(core, ea, result, wData, i8086_REG_DS);/*DEST-Mem schreiben*/
           }
}                        



/*Autor: RD       */
/*AND AX/AL,const */
/*TEST AX/AL,const*/
/*OR AX/AL,const  */
/*XOR AX/AL,const */
void i8086AndTestOrXorAccumulator(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SingleRegister ax;
  unsigned short op1,res=0;
  unsigned char wData;

  core->flags&=~i8086_FLAG_CF;  //Carry Flag 0 setzen
  core->flags&=~i8086_FLAG_OF;  //Overflow Flag 0 setzen

  wData = opcode & i8086_BIT_0;                     /* 1=16bit, 0=8bit  */
  i8086GetRegisterRef(&ax,core,wData,i8086_REG_AX);         /* Register holen   */

  if(wData) //16bit immediate
  {
    op1=joinBytes(para.b[1],para.b[0]);
  }
  else            //8bit immediate
  {
    op1=para.b[0];
  }

  //welche Operation?
  if (opcode==0x24 || opcode==0x25 || opcode==0xA8 || opcode==0xA9)	// and/test
	res=ax.x&op1;
  else if (opcode == 0x0C || opcode == 0x0D)		// or
	res=ax.x|op1;
  else if (opcode == 0x34 || opcode == 0x35)			// xor
  	res=ax.x^op1;

  if (opcode != 0xA8 && opcode != 0xA9) //wenns nicht Test ist
  	i8086SetRegister(core, i8086_REG_AX, wData, res);

  //FLAGS
  correctArithmeticFlags(core, res, wData);
}


/*Autor: RD           */
/*AND  Reg/Mem,Reg/Mem*/
/*TEST Reg/Mem,Reg/Mem*/
/*OR   Reg/Mem,Reg/Mem*/
/*XOR  Reg/Mem,Reg/Mem*/
void i8086AndTestOrXorRegMem(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
  i8086SingleRegister reg1,reg2;
  unsigned short result,operand;
  unsigned char wData;
  char dest,source;
  i808616BitAdr adr;

  wData = opcode & i8086_BIT_0;                     /* 1=16bit, 0=8bit           */

  core->flags&=~i8086_FLAG_CF;  //Carry Flag 0 setzen
  core->flags&=~i8086_FLAG_OF;  //Overflow Flag 0 setzen

  if ((para.b[0] & 192) == 192) //reg to reg
  {

    if(opcode & i8086_BIT_1) //Reg1 TO Reg2
    {
       source = getBitSnipped(para.b[0], 2, 3);/* Register-Code für Source   */
       dest = getBitSnipped(para.b[0], 5, 3);/* Register-Code für Dest */
    }
    else //Reg2 To Reg1
    {
        dest = getBitSnipped(para.b[0], 2, 3);/* Register-Code für Dest   */
        source = getBitSnipped(para.b[0], 5, 3);/* Register-Code für Source */
    }

    i8086GetRegisterRef(&reg1,core,wData,source);
    i8086GetRegisterRef(&reg2,core,wData,dest);

    //Operation ausführen
    if ((opcode&(~1))==0x84 || (opcode&(~3))==0x20)
    	result=reg1.x & reg2.x;	//and/test
    else if ((opcode&(~3))==0x08)
    	result=reg1.x | reg2.x;	//or
    else if ((opcode&(~3))==0x30)
    	result=reg1.x ^ reg2.x;	//xor

    if ((opcode&(~1))!=0x84)	//wenns nicht Test ist wird das Ergebnis geschrieben
    	i8086SetRegister(core,dest,wData,result);

  }
  else	// Reg to Mem / Mem to Reg
  {
    adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));  //speicheradresse decodieren
    dest= getBitSnipped(para.b[0], 5, 3);//destregister
    i8086GetRegisterRef(&reg1,core,opcode & 1,dest);
    operand=memRead(core,adr,opcode & 1,i8086_REG_DS);

     //Operation ausführen
    if ((opcode&(~1))==0x84 || (opcode&(~3))==0x20)
    	result=operand & reg1.x;	//and/test
    else if ((opcode&(~3))==0x08)
    	result=operand | reg1.x;	//or
    else if ((opcode&(~3))==0x30)
    	result=operand ^ reg1.x;	//xor


    if ((opcode&(~1))!=0x84)	//wenns nicht Test ist wird das Ergebnis geschrieben
    {
	    if (opcode&2) //from mem to reg
	    {
	       i8086SetRegister(core,dest,wData, result);
	    }
	    else  //from reg to mem
	    {
	       memWrite(core,adr,result,wData,i8086_REG_DS);
	    }
    }
  }

  correctArithmeticFlags(core,result,wData);
}

