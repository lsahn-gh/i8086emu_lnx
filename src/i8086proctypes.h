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
/* Datatypes for i8086proc.h                        */
/*                                                  */
/****************************************************/

#ifndef _i8086PROCTYPES_H_
#define _i8086PROCTYPES_H_

/* Register-Konstanten */
#define i8086_REG_AX 0 /* w = 1 */
#define i8086_REG_AL 0 /* w = 0 */
#define i8086_REG_AH 4 /* w = 0 */
#define i8086_REG_BX 3 /* w = 1 */
#define i8086_REG_BL 3 /* w = 0 */
#define i8086_REG_BH 7 /* w = 0 */
#define i8086_REG_CX 1 /* w = 1 */
#define i8086_REG_CL 1 /* w = 0 */
#define i8086_REG_CH 5 /* w = 0 */
#define i8086_REG_DX 2 /* w = 1 */
#define i8086_REG_DL 2 /* w = 0 */
#define i8086_REG_DH 6 /* w = 0 */

#define i8086_REG_SP 4 /* w = 1 */
#define i8086_REG_BP 5 /* w = 1 */
#define i8086_REG_SI 6 /* w = 1 */
#define i8086_REG_DI 7 /* w = 1 */

/* Segment-Register*/
#define i8086_REG_ES 0
#define i8086_REG_CS 1
#define i8086_REG_SS 2
#define i8086_REG_DS 3
#define i8086_SEGMENT0 4

/* Flags-Konstanten */
#define i8086_FLAG_CF  1    /* Carry-Flag */
#define i8086_FLAG_ACF 16   /* Auxillary-Carry-Flag */
#define i8086_FLAG_OF  2048 /* Overflow-Flag */
#define i8086_FLAG_SF  128  /* Sign-Flag */
#define i8086_FLAG_PF  4    /* Parity-Flag */
#define i8086_FLAG_ZF  64   /* Zerro-Flag */
#define i8086_FLAG_DF  1024 /* Direction-Flag */
#define i8086_FLAG_IF  512  /* Interrupt-Flag */
#define i8086_FLAG_TF  256  /* Trap-Flag */

/* Interne Konstanten */
/* Array-Position der Register */
#define i8086_AR_AX 0
#define i8086_AR_BX 1
#define i8086_AR_CX 2
#define i8086_AR_DX 3
#define i8086_AR_SP 4
#define i8086_AR_BP 5
#define i8086_AR_SI 6
#define i8086_AR_DI 7

#define i8086_AR_AL 0
#define i8086_AR_AH 1
#define i8086_AR_BL 2
#define i8086_AR_BH 3
#define i8086_AR_CL 4
#define i8086_AR_CH 5
#define i8086_AR_DL 6
#define i8086_AR_DH 7
#define i8086_AR_SPL 8
#define i8086_AR_SPH 9
#define i8086_AR_BPL 10
#define i8086_AR_BPH 11
#define i8086_AR_SIL 12
#define i8086_AR_SIH 13
#define i8086_AR_DIL 14
#define i8086_AR_DIH 15

#define i8086_FIRST_SEGREG 8 /* Index des ersten Segment-Registers im Register-Array */

/* Bit-Pos-Konstanten */
#define i8086_BIT_0 1
#define i8086_BIT_1 2
#define i8086_BIT_2 4
#define i8086_BIT_3 8
#define i8086_BIT_4 16
#define i8086_BIT_5 32
#define i8086_BIT_6 64
#define i8086_BIT_7 128

#define i8086_OPC_NAME_LEN 7 /* max. opcode-Namen-Laenge + \0 */

/* Misc */
#define i8086_CPU_SIG 0xF000; //Cpu ist ein 8086, die höchsten 4 bit der Flags müssen 1 sein
#define i8086_MEM_SIZE 65535
#define i8086_MAX_MEM_SEG 17 /* max. Anzahl an Speicher Segmenten. Ein Segment = i8086_MEM_SIZE Byte. */
#define i8086_PORTS 1025       /* Anzahl der Ports */
#define i8086_INPUT_PORT 0 /*es handelt sich um einen INPUT Port (verwendbar für portOpWord bzw. portOpByte in i8086util.c)*/
#define i8086_OUTPUT_PORT 1 /*es handelt sich um einen OUTPUT Port (verwendbar für portOpWord bzw. portOpByte in i8086util.c)*/
#define i8086_WRITE_PORT 1 /*es soll eine Schreiboperation ausgeführt werden (verwendbar für portOpWord bzw. portOpByte in i8086util.c)*/
#define i8086_READ_PORT 0 /*es soll eine Leseoperation ausgeführt werden (verwendbar für portOpWord bzw. portOpByte in i8086util.c)*/
#define i8086_BEGINADR 0x100   /* Anfangsposition des Programms im Speicher */

/* Cast fuer Speicherzugriffe. Wird die max. Adresse ueberschritten */
/* lauft der Wert ueber und beginnt wieder bei 0.                   */
#define CAST_TO_MEMSIZE(n) ((n)%((i8086_MEM_SIZE*i8086_MAX_MEM_SEG)+1))
/* Cast fuer eine beliebige Basis. */
#define CAST_TO_BASE(n, base) ((n)%(base))

//überprüft ob Opcode ein Befehlsprefix ist (cs,ds,es,ss,rep,lock)
#define isPrefix(opcode) (opcode == 0x2E ||opcode == 0x26 || opcode == 0x36 || opcode == 0x3E || opcode == 0xF0 || opcode == 0xF2 || opcode == 0xF3)
/* Typen */

typedef unsigned short i8086wRegister; /* Typ fuer ein Word-Register */
typedef unsigned char i8086bRegister;  /* Typ fuer ein Byte-Register */

typedef union i8086SingleRegister
{
  unsigned short x;
  unsigned char b[2]; /* 0=lByte, 1=hByte */
}i8086SingleRegister;

typedef union i8086Register
{
  i8086wRegister x[13]; /* Index >7 = Segment-Register */
  i8086bRegister b[26];
}i8086Register;

typedef union i8086Parameter
{
  long long all;
  int i[2];
  unsigned short w[4]; // 0=l
  unsigned char b[8];  // 0=l
}i8086Parameter;

typedef char i8086W;  /* Befehlsmodifikator.
                         wBit bei Befehlen mit word oder byte Parametern
                         1=word, 0=byte
                       */

typedef unsigned short i808616BitAdr; //16 Bit Adresse
typedef unsigned int i8086DoubleWord; //32 Bit Zahl
/* Structs */

/*Representiert die I/O Ports*/
typedef struct i8086IOPorts
{
  unsigned short x[i8086_PORTS*2]; //Ports untere i8086_PORTS=input und obere i8086_PORTS=output
}i8086IOPorts;

typedef struct i8086MiscData
{
  unsigned int processedClocks;       /* Counter für die zum Zeitpunkt benoetigten Takte(8086). */
  unsigned long long emuTime;         /* Zeit die der emu benoetigte um die Befehle(processedClocks) auszufuehren. */
  struct timeval startTime, endTime;  /* Timer fuer emu-Geschwindigkeitsmessung */
  signed char segoverride;		/* gibt bei aktiven Segmentoverride den Segmentcode an, sonst -1 */
}i8086MiscData;

/* Representiert Speicher, Register und Flags des 8086 */
typedef struct i8086core
{
  i8086Register reg;      /* Register */
  int flags;              /* Flags */
  i8086IOPorts ports;     /*IO Ports */
  unsigned short pc;        /* Programmcounter */
  unsigned char *mem;     /* Speicher(64k) */
  i8086MiscData miscData; /* Zusaetzliche Daten fuer den Programmablauf */
}i8086core;


/* Representiert einen 8086 Befehl */
typedef struct i8086command
{
  char name[i8086_OPC_NAME_LEN]; /* Befehlsname */
  unsigned char opcode;   /* Befehl */
  unsigned char size;     /* Groesse des Befehls mit Parametern in Byte    */
  unsigned char hasMod;   /* 1=Mod-Tag existiert, 0=kein Mod-Tag vorhanden */
  unsigned char clocks;   /* Takte des Befehls   */
  void (*func)(i8086core *core, unsigned char opcode, i8086Parameter para, i8086Parameter data);  /* Emulatorfunktion */
}i8086command;

#endif /* _i8086PROCTYPES_H_ */
