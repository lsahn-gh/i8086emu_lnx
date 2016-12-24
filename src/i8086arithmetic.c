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
/* Autor: RD                                        */
/* Version: 1.0                                     */
/*                                                  */
/* Arithmetikfunktionen		                          */
/*                                                  */
/****************************************************/

#include <stdlib.h>
#include "i8086proc.h"
#include "i8086util.h"
#include "i8086emufuncs.h"
#include "i8086controltrans.h"


//Inc für 16bit Register
void i8086IncReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	unsigned short sum;
	i8086SingleRegister *reg;
	reg=i8086GetRegister(core,1,opcode & 7);
	sum=reg->x+1;
	i8086SetRegister(core,opcode & 7,1,sum);

	correctArithmeticFlags(core,sum,1);
	correctOvCaFlagafterAddSub(core, reg->x, 1, sum, 1,1);

	free(reg);
}

//Inc und Dec für 8/16bit Register und Speicherzellen
void i8086IncDecRegMem(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *reg;
	unsigned short sum,summand=0;
	i808616BitAdr adr;
	unsigned char addsub;

	if (getBitSnipped(para.b[0],5,3)==0) 	//ist dec oder inc gemeint?
		{
			summand=1;				//inc
			addsub=1;
		}
		else
		{
			summand=-1;				//dec
			addsub=0;
		}

	if ((para.b[0] & 192) == 192) //Register
	{
		reg=i8086GetRegister(core,opcode & 1,para.b[0]&7);	//Registernummer

		sum=reg->x+summand;
		i8086SetRegister(core,para.b[0]&7,opcode & 1,sum);		//Register schreiben
		//Flags setzen
		correctOvCaFlagafterAddSub(core, reg->x, summand, sum, opcode & 1,addsub);
		free(reg);
	}
	else
	{	//Speicher
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));	//speicheradresse decodieren

		sum=memRead(core,adr,opcode & 1,i8086_REG_DS)+summand;

		//Flags setzen
		correctOvCaFlagafterAddSub(core, memRead(core,adr,opcode & 1,i8086_REG_DS), summand, sum, opcode & 1,addsub);
		memWrite(core,adr,sum,opcode & 1,i8086_REG_DS);	//ergebnis speichern
	}

	//Flags setzen
	correctArithmeticFlags(core,sum,opcode&1);
}

//Dec für 16bit Register
void i8086DecReg(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	unsigned short sum;
	i8086SingleRegister reg;
	
	i8086GetRegisterRef(&reg, core, 1, opcode & 7);
	sum=reg.x-1;
	i8086SetRegister(core, opcode & 7, 1, sum);

	correctArithmeticFlags(core, sum, 1);
	correctOvCaFlagafterAddSub(core, reg.x, -1, sum, 1,0);
}


//Add mit Konstante
void i8086AddImmed(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister reg;
	unsigned char al; //additional length
	i808616BitAdr adr;
	unsigned short sum,immeddata=0;
	unsigned char addsub=1;

	al=getAdditionalCmdLength(core, opcode, para.b[0]); //Länge von Disp

	if( opcode & 2) //8bit immediate
	{
		immeddata=para.b[al+1];
	}
	else //16bit immediate
	{
		immeddata=joinBytes(para.b[al+2],para.b[al+1]);
	}

	if ((para.b[0] & 192) == 192) //immediate to reg
	{
		i8086GetRegisterRef(&reg,core,opcode & 1,para.b[0]&7); //Zielregister

		if ((getBitSnipped(para.b[0],5,3)==2) && (core->flags & i8086_FLAG_CF))			//add with carry
			immeddata++;
			else if (getBitSnipped(para.b[0],5,3)==5 || getBitSnipped(para.b[0],5,3)==7 || getBitSnipped(para.b[0],5,3)==3)		//sub / sbb / cmp
			{
				immeddata=-immeddata;
				addsub=0;
				if (getBitSnipped(para.b[0],5,3)==3 && (core->flags & i8086_FLAG_CF))	//sbb
					immeddata--;
			}

		sum=reg.x+immeddata;

		if (getBitSnipped(para.b[0],5,3)!=7)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
			i8086SetRegister(core,para.b[0]&7,opcode & 1,sum);

		correctOvCaFlagafterAddSub(core, reg.x, immeddata, sum, opcode & 1,addsub);

	}
	else	//immediate to mem
	{
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));

		if ((getBitSnipped(para.b[0],5,3)==2) && (core->flags & i8086_FLAG_CF))			//add with carry
			immeddata++;
			else if (getBitSnipped(para.b[0],5,3)==5 || getBitSnipped(para.b[0],5,3)==7 || getBitSnipped(para.b[0],5,3)==3)		//sub / sbb / cmp
			{
				immeddata=-immeddata;
				addsub=0;
				if (getBitSnipped(para.b[0],5,3)==3 && (core->flags & i8086_FLAG_CF))	//sbb
					immeddata--;
			}

		sum=memRead(core,adr,opcode & 1,i8086_REG_DS)+immeddata;
		correctOvCaFlagafterAddSub(core, memRead(core,adr,opcode & 1,i8086_REG_DS), immeddata, sum, opcode & 1,addsub);

		if (getBitSnipped(para.b[0],5,3)!=7)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
			memWrite(core,adr,sum,opcode & 1,i8086_REG_DS);
	}

	correctArithmeticFlags(core,sum,opcode & 1);
}


//Add für den Akkumulator (AX/AL)
void i8086AddAcc(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister ax;
	unsigned short summand,sum=0;
	unsigned char addsub=1;
	i8086GetRegisterRef(&ax,core,1,i8086_REG_AX);

	if( opcode & 1) //16bit immediate
	{
		summand=joinBytes(para.b[1],para.b[0]);
	}
	else //8bit immediate
	{
		summand=para.b[0];
	}

	if ((getBitSnipped(opcode,7,7)==10) && (core->flags & i8086_FLAG_CF))			//add with carry
		summand++;
		else if (getBitSnipped(opcode,7,7)==22 || getBitSnipped(opcode,7,7)==30 || getBitSnipped(opcode,7,7)==14)		//sub / sbb / cmp
		{
			summand=-summand;
			addsub=0;
			 if (getBitSnipped(opcode,7,7)==14 && (core->flags & i8086_FLAG_CF))	//sbb
				summand--;
		}

	sum=ax.x+summand;

	if (getBitSnipped(opcode,7,7)!=30)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
	{	if (opcode & 1)
			i8086SetRegister(core,i8086_REG_AX,opcode & 1,sum);
		else
			i8086SetRegister(core,i8086_REG_AL,opcode & 1,sum);
	}

	correctOvCaFlagafterAddSub(core, ax.x, summand, sum, opcode & 1,addsub);
	correctArithmeticFlags(core,sum,opcode&1);
}

//add adc sbb sub cmp sind sich sehr ähnlich, deswegen alle in einer Funktion
//implementiert Commandos add reg/mem,reg/mem, sub reg/mem,reg/mem, sbb reg/mem,reg/mem, adc reg/mem,reg/mem, cmp reg/mem,reg/mem
void i8086AddRegMem(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister reg1,reg2;
	unsigned short sum,summand=0;
	char dest,source;
	i808616BitAdr adr;
	unsigned char addsub=1;
	unsigned char subopc=getBitSnipped(opcode,7,6);

	source=(para.b[0]&56)>>3;					//registernummer holen
	i8086GetRegisterRef(&reg1,core,opcode & 1,source);	//ein Parameter ist immer ein Register

	if ((para.b[0] & 192) == 192) //reg to reg
	{

		dest=para.b[0]&7;

		//DIRECTION FLAG auswerten
		if (opcode&2)
		{
			i8086GetRegisterRef(&reg1,core,opcode & 1,dest);
			dest=source;
		}

		i8086GetRegisterRef(&reg2,core,opcode & 1,dest);

		summand=reg1.x;

		if ((subopc==4) && (core->flags & i8086_FLAG_CF))			//add with carry
					summand++;
				else if (subopc==10 || subopc==14 || subopc==6)		//sub / sbb / cmp
				{
					summand=-summand;
					addsub=0;
					if (subopc==6 && (core->flags & i8086_FLAG_CF))	//sbb
						summand--;
				}

		sum=reg2.x+summand;

		if (subopc!=14)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
			i8086SetRegister(core,dest,opcode & 1,sum);

		correctOvCaFlagafterAddSub(core, reg2.x, summand, sum, opcode & 1,addsub);
	}
	else
	{
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));	//speicheradresse decodieren

		if (opcode&2) //mem to reg
		{
				summand=memRead(core,adr,opcode & 1,i8086_REG_DS);

				if ((subopc==4) && (core->flags & i8086_FLAG_CF))			//add with carry
					summand++;
					else if (subopc==10 || subopc==14 || subopc==6)		//sub / sbb / cmp
					{
						summand=-summand;
						addsub=0;
						 if (subopc==6 && (core->flags & i8086_FLAG_CF))	//sbb
							summand--;
					}

				sum=reg1.x+summand;

				if (subopc!=14)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
					i8086SetRegister(core,source,opcode & 1, sum);

				correctOvCaFlagafterAddSub(core, reg1.x, summand, sum, opcode & 1,addsub);
		}
		else	//from reg to mem
		{
				summand=reg1.x;

				if ((subopc==4) && (core->flags & i8086_FLAG_CF))			//add with carry
					summand++;
					else if (subopc==10 || subopc==14 || subopc==6)		//sub / sbb / cmp
					{
						summand=-summand;
						addsub=0;
						if (subopc==6 && (core->flags & i8086_FLAG_CF))	//sbb
							summand--;
					}

				sum=memRead(core,adr,opcode & 1,i8086_REG_DS)+summand;
				correctOvCaFlagafterAddSub(core, memRead(core,adr,opcode & 1,i8086_REG_DS), summand, sum, opcode & 1,addsub);

				if (subopc!=14)	//wenns nicht cmp ist dann wird das ergebnis geschrieben
					memWrite(core,adr,sum,opcode & 1,i8086_REG_DS);
		}
	}

	correctArithmeticFlags(core,sum,opcode&1);
}


//cbw
//konvertiert signed zahl in al zu signed zahl in ax
void i8086ConvertByteWord(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	if (core->reg.b[i8086_AR_AL] & 0x80)
		core->reg.b[i8086_AR_AH]=0xFF;
	else
		core->reg.b[i8086_AR_AH]=0x00;
}

//cwd
//konvertiert signed zahl in ax zu signed 32bit zahl in ax und dx
void i8086ConvertByteDWord(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	if (core->reg.x[i8086_AR_AX] & 0x8000)
		core->reg.x[i8086_AR_DX]=0xFFFF;
	else
		core->reg.x[i8086_AR_DX]=0x0000;
}


//neg
//negiert Register/Speicherzelle
void i8086Negate(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i808616BitAdr adr;
	i8086SingleRegister *reg;
	unsigned short result;

	if ((para.b[0] & 192) == 192) //reg
	{
		reg=i8086GetRegister(core,opcode & 1,para.b[0]&7);	//Registernummer
		result=-reg->x;

		i8086SetRegister(core,para.b[0]&7,opcode & 1,result);		//Register schreiben
		free(reg);
	}
	else
	{
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));	//speicheradresse decodieren
		result=-memRead(core,adr,opcode & 1,i8086_REG_DS);

		memWrite(core,adr,result,opcode & 1,i8086_REG_DS);	//ergebnis speichern
	}

	correctOvCaFlagafterAddSub(core, 0, result, result, opcode & 1,0);
	correctArithmeticFlags(core,result,opcode&1);
}

//implementiert mul und imul
//multipliziert al/ax mit 8/16bit Inhalt des Parameters und schreibt in AX/DX:AX
void i8086Multiplicate(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

	i808616BitAdr adr;
	i8086SingleRegister *reg;
	unsigned short op1,op2,result16;
	i8086DoubleWord result32;
	unsigned char negresult=0;

	if ((para.b[0] & 192) == 192) //reg
	{
		reg=i8086GetRegister(core,opcode & 1,para.b[0]&7);	//Registernummer
		op2=reg->x;
		free(reg);
	}
	else
	{
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));	//speicheradresse decodieren
  		op2=memRead(core,adr,opcode & 1,i8086_REG_DS);
	}

	if (opcode & 1)	//32 bit ergebnis
		{
			reg=i8086GetRegister(core,1,i8086_REG_AX);
			op1=reg->x;
			free(reg);

			if (getBitSnipped(para.b[0], 5, 3)==5)	//bei imul wird erstmal mit postiven operanden gerechnet
			{
				negresult=(op1 & 32768 ) != (op2 & 32768);	//bei ungleichen vorzeichen muss ergebnis negiert werden
				if (op1 & 32768 ) op1=-op1;
				if (op2 & 32768 ) op2=-op2;
			}

			result32=op1*op2;

			if  (negresult)
				result32=-result32;

			// CF und OF werden gesetzt wenn der HIGH-Teil des Ergebnisses nicht eine vorzeichengerechte Erweiterung des LOW-Teils ist
			// ansonsten werden CF und OF gelöscht
			if ((result32 >> 15) == 0 || (result32 >> 15) == 131071 )
			{
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags & (~i8086_FLAG_OF);
			}
			else
			{
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags | i8086_FLAG_OF;
			}

			i8086SetRegister(core,i8086_REG_AX,1,result32);		//Register AX schreiben
			i8086SetRegister(core,i8086_REG_DX,1,result32>>16);		//Register DX schreiben
		}
	else		//16 bit ergebnis
		{
			reg=i8086GetRegister(core,0,i8086_REG_AL);
			op1=reg->b[0];
			free(reg);

			if (getBitSnipped(para.b[0], 5, 3)==5)	//bei imul wird erstmal mit postiven operanden gerechnet
			{
				negresult=(op1 & 128 ) != (op2 & 128);	//bei ungleichen vorzeichen muss ergebnis negiert werden
				if (op1 & 128 ) op1=(unsigned char)-op1;
				if (op2 & 128 ) op2=(unsigned char) -op2;
			}

			result16=op1*op2;

			if  (negresult )
				result16=-result16;

			if ((result16 & 65408) == 0 || (result16 & 65408) == 65408)
			{
				core->flags=core->flags & (~i8086_FLAG_CF);
				core->flags=core->flags & (~i8086_FLAG_OF);
			}
			else
			{
				core->flags=core->flags | i8086_FLAG_CF;
				core->flags=core->flags | i8086_FLAG_OF;
			}

			i8086SetRegister(core,i8086_REG_AX,1,result16);		//Register AX schreiben
		}

}

//implementiert div und idiv
//dividiert AX/DX:AX mit 8/16bit Inhalt vom Operator, der Quotient steht in AL/AX und der Rest in AH/DX
void i8086Divide(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{

	i808616BitAdr adr;
	i8086SingleRegister reg;
	unsigned short op1_16,op2,result;
	i8086DoubleWord op1_32,result32;

	if ((para.b[0] & 192) == 192) //reg
	{
		i8086GetRegisterRef(&reg,core,opcode & 1,para.b[0]&7);	//Registernummer
		if (opcode & 1)
			op2=reg.x;
		else
			op2=reg.b[0];
	}
	else
	{
		adr=decodeMemAdr(core, para.b[0], joinBytes(para.b[1],para.b[2]));	//speicheradresse decodieren
		op2=memRead(core,adr,opcode & 1,i8086_REG_DS);
	}

	if (op2==0)		//Division durch 0
	{
		i8086callInterrupt(core,0);
		return;
	}


	if (opcode & 1)	//32 bit divisor
		{
			i8086GetRegisterRef(&reg,core,1,i8086_REG_AX);
			op1_32=reg.x;

			i8086GetRegisterRef(&reg,core,1,i8086_REG_DX);
			op1_32=op1_32| reg.x<<16;


			if (getBitSnipped(para.b[0], 5, 3)==7)	//bei idiv wird mit signed operanden gerechnet
			{
				result32=(signed) ((i8086DoubleWord) op1_32) / (signed short) op2;
				i8086SetRegister(core,i8086_REG_DX,1, (signed) ((i8086DoubleWord) op1_32) % (signed short) op2);		//Register DX schreiben
			}
			else
			{
				result32=op1_32/op2;
				i8086SetRegister(core,i8086_REG_DX,1,op1_32%op2);		//Register DX schreiben
			}

			if ((signed) result32>32768 || (signed) result32<-32767)	//falls Ergebnis zu gross für den Accu wird int 0 ausgelöst
			{
				i8086callInterrupt(core,0);
				return;
			}

			i8086SetRegister(core,i8086_REG_AX,1,result32);		//Register AX schreiben
		}
	else		//16 bit divisor
		{
			i8086GetRegisterRef(&reg,core,1,i8086_REG_AX);
			op1_16=reg.x;

			if (getBitSnipped(para.b[0], 5, 3)==7)	//bei idiv wird mit signed operanden gerechnet
			{
				result=(signed short) op1_16/ (signed char) op2;
				i8086SetRegister(core,i8086_REG_AH,0,((signed short) op1_16) % ((signed char) op2));		//Rest in Register AH schreiben
			}
			else
			{
				result=op1_16/ op2;
				i8086SetRegister(core,i8086_REG_AH,0,op1_16%op2);		//Rest in Register AH schreiben
			}

			if (((signed short)result)>128 || ((signed short)result) <-127)	//falls Ergebnis zu gross für den Accu wird int 0 ausgelöst
			{
				i8086callInterrupt(core,0);
				return;
			}

			i8086SetRegister(core,i8086_REG_AL,0,result);		//Quotient in Register AL schreiben
		}
}

/*
Die Algorithmik folgender Funktionen ist in diversen Büchern nachzulesen (siehe Quellen)
*/

//AAA
//ASCII Adjust after Addition
void i8086AAA(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	if ((ax->b[0] & 0x0F) > 9 || (core->flags & i8086_FLAG_ACF))
	{
		//AL = AL + 6
		//i8086SetRegister(core,i8086_REG_AL,0,ax->b[0]+6);
		ax->b[0]+=6;
		//AH = AH + 1
		i8086SetRegister(core,i8086_REG_AH,0,ax->b[1]+1);
		core->flags=core->flags | i8086_FLAG_ACF;
		core->flags=core->flags | i8086_FLAG_CF;
	}
	else
	{
		core->flags=core->flags & (~i8086_FLAG_ACF);
		core->flags=core->flags & (~i8086_FLAG_CF);
		//AL = AL & 0Fh
		//i8086SetRegister(core,i8086_REG_AL,0,ax->b[0] & 0x0F);
	}
	i8086SetRegister(core,i8086_REG_AL,0,ax->b[0] & 0x0F);
	free(ax);
}

//AAD
//ASCII Ajust before Division
void i8086AAD(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	//AL = AH * 10 + AL
	i8086SetRegister(core,i8086_REG_AL,0,ax->b[1]*10+ax->b[0]);
	//AH = 0
	i8086SetRegister(core,i8086_REG_AH,0,0);

	correctArithmeticFlags(core,ax->b[1]*10+ax->b[0],0);

	free(ax);
}

//AAM
//ASCII Adjust after Multiply
void i8086AAM(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	//AH = AL / 10
	i8086SetRegister(core,i8086_REG_AH,0,ax->b[0]/10);
	//AL = AL MOD 10
	i8086SetRegister(core,i8086_REG_AL,0,ax->b[0]%10);

	free(ax);
	ax=i8086GetRegister(core,1,i8086_REG_AX);
	correctArithmeticFlags(core,ax->x,1);

	free(ax);
}

//AAS
//ASCII Adjust after Subtraction
void i8086AAS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	if ((ax->b[0] & 0x0F) > 9 || (core->flags & i8086_FLAG_ACF))
	{
		//AL = AL - 6
		//i8086SetRegister(core,i8086_REG_AL,0,ax->b[0]-6);
		ax->b[0]-=6;
		//AH = AH - 1
		i8086SetRegister(core,i8086_REG_AH,0,ax->b[1]-1);
		core->flags=core->flags | i8086_FLAG_ACF;
		core->flags=core->flags | i8086_FLAG_CF;
	}
	else
	{
		core->flags=core->flags & (~i8086_FLAG_ACF);
		core->flags=core->flags & (~i8086_FLAG_CF);
		//AL = AL & 0Fh
		//i8086SetRegister(core,i8086_REG_AL,0,ax->b[0] & 0x0F);
	}
	i8086SetRegister(core,i8086_REG_AL,0,ax->b[0] & 0x0F);
	free(ax);
}

//DAA
//Decimal Adjust after Addition
void i8086DAA(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	// If (AL & 0Fh) > 9 or (AF = 1)
	if ((ax->b[0] & 0x0F) > 9 || (core->flags & i8086_FLAG_ACF))
	{
		//AL = AL + 6
		ax->b[0]+=6;
		//AF = 1
		core->flags=core->flags | i8086_FLAG_ACF;

		/*if (ax->b[0]>=0 && ax->b[0]<6)
		core->flags|=i8086_FLAG_CF;
		else
		core->flags&=~i8086_FLAG_CF;*/
	}
	else
		//AF = 0
		core->flags=core->flags & (~i8086_FLAG_ACF);

	// If (AL > 9Fh) or (CF = 1)
	if (ax->b[0] > 0x9f || (core->flags & i8086_FLAG_CF))
	{
		//AL = AL + 60h
		ax->b[0]+=0x60;
		//CF=1
		core->flags=core->flags | i8086_FLAG_CF;
	}
	else
		//CF=0
		core->flags=core->flags & (~i8086_FLAG_CF);

	i8086SetRegister(core,i8086_REG_AL,0,ax->b[0]);
	correctArithmeticFlags(core,ax->x,1);
	free(ax);
}

//DAS
//Decimal Adjust after Subtraction
void i8086DAS(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data)
{
	i8086SingleRegister *ax;
	ax=i8086GetRegister(core,1,i8086_REG_AX);

	// If (AL & 0Fh) > 9 or (AF = 1)
	if ((ax->b[0] & 0x0F) > 9 || (core->flags & i8086_FLAG_ACF))
	{
		//AL = AL - 6
		ax->b[0]-=6;
		//AF = 1
		core->flags=core->flags | i8086_FLAG_ACF;
	}
	else
		//AF = 0
		core->flags=core->flags & (~i8086_FLAG_ACF);

	// If (AL > 9Fh) or (CF = 1)
	if (ax->b[0] > 0x9f || (core->flags & i8086_FLAG_CF))
	{
		//AL = AL - 60h
		ax->b[0]-=0x60;
		//CF=1
		core->flags=core->flags | i8086_FLAG_CF;
	}
	else
		//CF=0
		core->flags=core->flags & (~i8086_FLAG_CF);

	i8086SetRegister(core,i8086_REG_AL,0,ax->b[0]);
	correctArithmeticFlags(core,ax->x,1);
	free(ax);
}
