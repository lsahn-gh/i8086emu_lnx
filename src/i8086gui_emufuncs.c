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
/****************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <ctype.h>
#include "i8086gui.h"
#include "i8086gui_emufuncs.h"
#include "i8086gui_util.h"
#include "i8086proc.h"
#include "i8086error.h"
#include "i8086config.h"
#include "i8086gui_error.h"
#include "i8086messages.h"
#include "i8086gui_paint.h"
#include "i8086util.h"
#include "i8086emufuncs.h"


// unsigned short startAdr=0x0100;
char canRun;

/* Anzahl der bereits gemachten Schleifendurchlaeufe */
/* im Run-Modus */
unsigned int clocks=0;
unsigned int debugger_update_interval=0;

//enthält Tastencode aktuell gedrückter Taste an der emulierten Tastatur, 0 <> keine
unsigned char currentKeyboardKey=0;
i8086msgFunc *guiOldPortSignalOutHandler=NULL;


//Autor Prof. Geiler
//aus Skript
char *stringcat(const char *s, ...)
{
	/*-----------------------------------------------------
	Funktion verkettet alle als Parameter übergebenen
	Zeichenketten und gibt das Ergebnis in einem
	dynamischen Feld zurück. Endekennung: NULL-Parameter.
	Der Caller muss den dyn. Speicher wieder freigeben !
	Bsp:
	char *erg=stringcat("Hallo ", "Ihr ", "da!", NULL);
	-----------------------------------------------------*/
	int len = strlen(s) + 1; /* erste Teilkette */
	char *result = (char *)malloc(len); /* für Ergebnis */
	char *substr; /* Zeiger für Parameter */
	va_list vlist; /* variable Argumentliste vorbereiten */
	strcpy(result, s); /* erste Kette kopieren */
	va_start(vlist, s); /* variable Liste starten */

	for(;;) 
	{
		substr=va_arg(vlist, char *); /* nächster Parameter */
		if(substr == NULL) break; /* NULL -> fertig! */
		len=len+strlen(substr); /* neue Länge */
		result = (char *)realloc(result, len); /* reservieren */
		strcat(result, substr); /* Parameter anhängen */
	}
	va_end(vlist);
	
	return result;
}


void guiPortSignalOutHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
  #define DISPLAY_REFRESH_RATE 1000
  static unsigned char oldSegments[8] = {0,0,0,0,0,0,0,0};
  static unsigned char oldLeds=0;
  unsigned char segment;

  if (hParam==0)
  {
    if (oldLeds!=lParam)
    {
      oldLeds=lParam;
      paint_leds(leds, NULL);
    }
  }
	else if (hParam>=0x90 && hParam<=0x9E)
	{
		segment=(15-(hParam-0x90))/2;
		if (oldSegments[segment]!=lParam)
		{
		  paint_display_segment(anz, lParam, segment);
		  oldSegments[segment]=lParam;
	  }
	}
		
	if (guiOldPortSignalOutHandler)
	  guiOldPortSignalOutHandler(mtyp, hParam, lParam);
}

/*
ist zu CPU-lastig
void guiPortSignalInHandler(unsigned short mtyp, unsigned int hParam, unsigned int lParam)
{
	 if (hParam==0x80);		//0x80 Tastaturport
	{
		//simuliert Verhalten der Sbc-86 Tastatur
		if (currentKeyboardKey==0)
			core->ports.x[0x80]=(core->ports.x[0x80]+8) | 7;
	}
}
*/

void i8086guiInitEmu()
{
  canRun = 0;
  //shutdown = 0;
  i8086clearLog(); /* Log-File leeren. */
  i8086init(); /* Prozessor initialisieren. */
  /* Eventhandler */

  guiOldPortSignalOutHandler = i8086SetMsgFunc(i8086_SIG_PORT_OUT, guiPortSignalOutHandler);
  //i8086SetMsgFunc(i8086_SIG_PORT_IN, guiPortSignalInHandler);

  /* Config-File laden */
  i8086guiLoadConfig(CONFIG_FILE);
}

void i8086guiCloseEmu()
{
  i8086close(); /* MsgQueue schliessen */
  guiOldPortSignalOutHandler = NULL;
}

/* Autor: RD */
/* sperrt die Buttons und Menüeinträge für die Programmausführung */
void i8086gui_disable_run_buttons()
{
  gtk_widget_set_sensitive(runbutton, FALSE);
  gtk_widget_set_sensitive(tracebutton, FALSE);
  gtk_widget_set_sensitive(stepoverbutton, FALSE);

  gtk_widget_set_sensitive(runmenuitem, FALSE);
  gtk_widget_set_sensitive(stepovermenuitem, FALSE);
  gtk_widget_set_sensitive(tracemenuitem, FALSE);
}

/* Autor: RD */
/* reaktiviert die Buttons und Menüeinträge für die Programmausführung */
void i8086gui_enable_run_buttons()
{
  gtk_widget_set_sensitive(runbutton, TRUE);
  gtk_widget_set_sensitive(tracebutton, TRUE);
  gtk_widget_set_sensitive(stepoverbutton, TRUE);

  gtk_widget_set_sensitive(runmenuitem, TRUE);
  gtk_widget_set_sensitive(stepovermenuitem, TRUE);
  gtk_widget_set_sensitive(tracemenuitem, TRUE);
}

void i8086guiLoadBinFile(const char *filename, unsigned int adr)
{
  #define STR_SUC_LOADED " successfully loaded"
  char *msg;
  
  i8086guiStop(NULL, NULL); /* evtl. laufenden ROM beenden */
  i8086reset();
  i8086SetRegister(core, i8086_REG_SP, 1, 0x100);
  
  if (i8086loadBinFile(core, filename, adr)!=i8086_SUC_FILELOAD)
  {
    i8086guiMessage(window, filename, i8086_ERR_STR_FILENF, GTK_MESSAGE_ERROR);
  }
  else
  {
    core->pc = adr;
    msg = (char*)calloc(strlen(filename)+strlen(STR_SUC_LOADED)+1, sizeof(char));
    strcpy(msg, filename);
    strcat(msg, STR_SUC_LOADED);
    showStatusbarMessage(msg);
    free(msg);
  }
  i8086guiPaintAll();//gui nach dem reset zeichnen
}

void i8086guiLoadConfig(const char *filename)
{
  char cfgStr[i8086_CFG_MAX_VALUE_LEN];
  unsigned int rom_adr;

  if (i8086ReadStrConfig(cfgStr, CONFIG_FILE, "ROMFILE")!=0)
  {
    rom_adr = i8086ReadHexConfig(CONFIG_FILE, "ROMSTARTADR", 0xc000);
    i8086loadBinFile(core, cfgStr, rom_adr);
    //core->pc = i8086ReadHexConfig(CONFIG_FILE, "ROMSTARTADR", 0xc000);
  }
  else /* Wenn ROM -> kein Core-Dump */
    if (i8086ReadStrConfig(cfgStr, CONFIG_FILE, "COREDUMP")!=0)
      i8086loadBinFile(core, cfgStr, 0);    
}

void i8086guiNextCommand(GtkWidget *button, gpointer data)
{
  char str[6];
  
  i8086guiStop(NULL, NULL);

  if (!canRun && i8086execCommand(core, commands)==i8086_ERR_ILGOPCODE)
  {
    sprintf(str, "%hd", core->mem[core->pc + i8086GetSegRegister_fast(core, i8086_REG_CS, 1)]);
    i8086guiError(window, i8086_ERR_STR_ILGOPCODE, str);
  }
  else
  {
    i8086guiPaintAll(); 
  }
}

void i8086guiRun(GtkWidget *button, i808616BitAdr data)
{
	#define WATCH 5000
	char str[6];
	canRun = 1;

  showStatusbarMessage("Running");
  i8086gui_disable_run_buttons();

  while (canRun && i8086GetSegRegister_fast(core,i8086_REG_CS, 1)+core->pc!=data)
  {
  
    if (i8086execCommand(core, commands)==i8086_ERR_ILGOPCODE)
    {
      sprintf(str, "%hd", core->mem[core->pc + i8086GetSegRegister_fast(core, i8086_REG_CS, 1)]);
      i8086guiError(window, i8086_ERR_STR_ILGOPCODE, str);
    }
    if(isBreakpoint(core->pc+i8086GetSegRegister_fast(core,i8086_REG_CS, 1)))//falls der naechste befehl an einem breakpoint liegt wird der emu gestoppt
   {
            i8086guiStop(NULL, NULL);
            break;
   }
    //simuliert Verhalten der Sbc-86 Tastatur
    if (currentKeyboardKey==0)
      core->ports.x[0x80]=(core->ports.x[0x80]+8) | 7;

    if (clocks%WATCH==0)
      while (gtk_events_pending())
	      gtk_main_iteration();
    clocks++;

    if (debugger_update_interval != 0 && clocks%debugger_update_interval==0)
      i8086guiPaintAll();
  }

  i8086guiPaintAll();
  //if (shutdown)
    //g_signal_emit_by_name(window, "delete-event");
    //destroy(NULL, NULL, NULL);
}

void i8086guiStop(GtkWidget *button, gpointer data)
{
  canRun = 0;
  showStatusbarMessage("Stopped");
  i8086gui_enable_run_buttons();
}

//Autor FB
//Funktionen zum anzeigen der Register
void i8086guiPaintAllRegister(GtkWidget *widget)
{
   i8086SingleRegister regbuf;
   unsigned short regData;
 	 char *text;//="";
	 char r[20];
	text=(char*)calloc(1, sizeof(char));

   //AX ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_AX);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "AX\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //BX ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_BX);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\tBX\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
   //CX ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_CX);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\nCX\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //DX ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_DX);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\tDX\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
   //SP ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_SP);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\nSP\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //BP ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_BP);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\tBP\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
   //SI ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_SI);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\nSI\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //DI ausgeben
   	i8086GetRegisterRef(&regbuf, core, 1, i8086_REG_DI);//Register Inhalt holen
   	regData=regbuf.x;
   	sprintf(r, "\tDI\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
   //CS ausgeben
    regData=i8086GetSegRegister_fast(core, i8086_REG_CS, 0);//Register Inhalt holen
   	sprintf(r, "\nCS\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //DS ausgeben
    regData=i8086GetSegRegister_fast(core, i8086_REG_DS, 0);//Register Inhalt holen
   	sprintf(r, "\tDS\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
   //SS ausgeben
    regData=i8086GetSegRegister_fast(core, i8086_REG_SS, 0);//Register Inhalt holen
   	sprintf(r, "\nSS\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b>", NULL);
   //ES ausgeben
    regData=i8086GetSegRegister_fast(core, i8086_REG_ES, 0);//Register Inhalt holen
   	sprintf(r, "\tES\t%04x", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);
	 //ES ausgeben
    regData=core->pc;//Register Inhalt holen
   	sprintf(r, "\nIP\t%04x    ", regData);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);

	sprintf(r, "\tFL\t%04x", core->flags);
	 	text=stringAdd(text,"<b>",r,"</b> ", NULL);

  gtk_label_set_markup (GTK_LABEL (widget),text);//Text auf das Label setzen

  free(text);

}



//Autor FB
//ruft die wirkliche flag mal Funktion auf
void i8086guiPaintAllFlags(GtkWidget *widget)
{
	char *text;
	text=(char*)calloc(1, sizeof(char));

	//wenn Flag gesetzt dann mal es rot ansonsten normal

	if(core->flags&i8086_FLAG_OF)
		text=stringAdd(text,"<span color=\"red\"><b>O</b></span>",NULL);
	else
		text=stringAdd(text,"<b>O</b>",NULL);

	text=stringAdd(text,"<b>:</b>",NULL);
	if(core->flags&i8086_FLAG_DF)
		text=stringAdd(text,"<span color=\"red\"><b>D</b></span>",NULL);
	else
		text=stringAdd(text,"<b>D</b>",NULL);

	text=stringAdd(text,"<b>:</b>",NULL);
	if(core->flags&i8086_FLAG_IF)
		text=stringAdd(text,"<span color=\"red\"><b>I</b></span>",NULL);
	else
		text=stringAdd(text,"<b>I</b>",NULL);

	text=stringAdd(text,"<b>:</b>",NULL);
	if(core->flags&i8086_FLAG_TF)
		text=stringAdd(text,"<span color=\"red\"><b>T</b></span>",NULL);
	else
		text=stringAdd(text,"<b>T</b>",NULL);

	text=stringAdd(text,"<b>:</b>",NULL);
	if(core->flags&i8086_FLAG_SF)
		text=stringAdd(text,"<span color=\"red\"><b>S</b></span>",NULL);
	else
		text=stringAdd(text,"<b>S</b>",NULL);

	text=stringAdd(text,"<b>:</b>",NULL);
	if(core->flags&i8086_FLAG_ZF)
		text=stringAdd(text,"<span color=\"red\"><b>Z</b></span>",NULL);
	else
		text=stringAdd(text,"<b>Z</b>",NULL);

	text=stringAdd(text,"<b>:x:</b>",NULL);
	if(core->flags&i8086_FLAG_ACF)
		text=stringAdd(text,"<span color=\"red\"><b>A</b></span>",NULL);
	else
		text=stringAdd(text,"<b>A</b>",NULL);

	text=stringAdd(text,"<b>:x:</b>",NULL);
	if(core->flags&i8086_FLAG_PF)
			text=stringAdd(text,"<span color=\"red\"><b>P</b></span>",NULL);
	else
		text=stringAdd(text,"<b>P</b>",NULL);

	text=stringAdd(text,"<b>:x:</b>",NULL);
	if(core->flags&i8086_FLAG_CF)
		text=stringAdd(text,"<span color=\"red\"><b>C</b></span>",NULL);
	else
		text=stringAdd(text,"<b>C</b>",NULL);

	//Labeltext setzen
	gtk_label_set_markup (GTK_LABEL (widget),text);

	free(text);
}

/* Autor: RD */
/* Handler für die Keyboard-Buttons */
 void i8086gui_keyboard_button_pressed (GtkButton *button, gpointer user_data)
 {
	currentKeyboardKey=(unsigned int) user_data;

	core->ports.x[0x80]=currentKeyboardKey;
 }

/* Autor: RD */
/* Handler für die Keyboard-Buttons */
/* wird benötigt falls das Keyboard per Tastatur bedient wird oder per Shortcuts, simuliert einen Tastendruck von 100ms */
 void i8086gui_keyboard_button_clicked (GtkButton *button, gpointer user_data)
 {
	currentKeyboardKey=(unsigned int) user_data;
	core->ports.x[0x80]=currentKeyboardKey;
	gtk_timeout_add (100, (GSourceFunc) i8086gui_keyboard_button_released, NULL);
 }

/* Autor: RD */
/* Handler für die Keyboard-Buttons */
 char i8086gui_keyboard_button_released (GtkButton *button, gpointer user_data)
 {
 	currentKeyboardKey=0;
	//Timeout beenden
	return FALSE;
 }

void i8086gui_cpu_reset (GtkButton *button, gpointer user_data)
{
	core->pc=0;
	//i8086reset();
	i8086SetSegRegister(core,i8086_REG_CS,0xc00);
	//i8086SetRegister(core,i8086_REG_AX,1,0);
	i8086guiRun(NULL,0xFFFF);
}

/* Autor: RD */
/* Handler für die Schalter (0-7) */
void i8086gui_on_switch_toggled (GtkButton *button, unsigned char user_data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (button)))
		core->ports.x[0]|=(1<<user_data);
	else
		core->ports.x[0]&=~(1<<user_data);
}

/* Autor: RD */
/* Handler den Interval Regler */
void i8086gui_adjust_update_interval(GtkRange *range, gpointer user_data)
{
	//Interval Regler im Bereich 0-50, 50 bedeutet kein Update
	if (gtk_range_get_value(GTK_RANGE(range))==50)
		//bei Interval 0 wird die Anzeige nicht aktualisiert
		debugger_update_interval=0;
	else
		//Reglerwert wird in einen brauchbaren Wertebereich konvertiert
		debugger_update_interval=(gtk_range_get_value(GTK_RANGE(range))+3)*3000;
}

/* Autor: RD */
/* StepOver Funktion */
void i8086guiStepOver(GtkWidget *button, gpointer data)
{
  i808616BitAdr nextcmdadr;
  i808616BitAdr adr;
  unsigned char opcode;

  adr=i8086GetSegRegister_fast(core,i8086_REG_CS, 1)+core->pc;
  opcode=core->mem[adr];

  //Der Emulator unterbricht die Ausführung nach dem nächsten Befehl, übergeht jedoch calls, loops, jcxz und ints
  //d.h. die einzelnen Befehle der Subroutinen etc. werden nicht unterbrochen
  if ( /*loop/jcxz*/ (opcode>=224 && opcode<=227) || /*int*/ (opcode>=204 && opcode<=206) || /*call*/ opcode == 232 || /*call*/ opcode == 154 || /*call*/ opcode == 255)
  {
  	nextcmdadr=adr+commands[opcode]->size;
   	if (commands[opcode]->hasMod!=0)
  	nextcmdadr+=getAdditionalCmdLength(core, opcode, core->mem[adr+1]);

  	i8086guiRun(NULL, nextcmdadr);
  	showStatusbarMessage("Stopped");
  	i8086gui_enable_run_buttons();
  }
  else
  {
  	i8086guiNextCommand(NULL, NULL);
  }
}

void i8086guiPaintAll()
{
  i8086guiPaintAllRegister(reglabel);
  i8086guiPaintAllFlags(flaglabel);
  paint_mem(memviewer, NULL, core, gtk_spin_button_get_value_as_int((GtkSpinButton*)memSpinButton));
  paint_disasm(disasmviewer, NULL, core, commands, 0);
}

/*Autor: CST*/
/*Schliesst das dialog fenster auf das mit user_data referenziert wird*/
void i8086event_closeDialog(GtkWidget *button, gpointer user_data)
{
      gtk_widget_destroy ((GtkWidget *) user_data);
}

/*Autor: CST*/
/*zeigt den aktuell markierten breakpoint im eingabefenster an*/
void i8086event_rowchanged(GtkTreeView *treeview, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *)store;  
  GtkTreeIter  iter;  
  GtkTreePath *path;  
  GtkTreeViewColumn *focus_column;
  gchar *bp;  /* get toggled iter */  
   enum 
   {
            NUMBER_COL,
            BP_COL,
            N_COLUMNS
   };
  gtk_tree_view_get_cursor(treeview,&path, &focus_column);//cursor im treeview bestimmen
  gtk_tree_model_get_iter (model, &iter, path);//zugehoerige reihe
  gtk_tree_model_get (model, &iter, BP_COL, &bp, -1); //wert des angeklickten breakpoints
  gtk_entry_set_text(GTK_ENTRY(bpadrtextbox),bp);//diesen wert in das eingabefeld schreiben
  selectedRow=(char *)bp;//den momentan ausgewaehlten breakpoint aktualisieren
  gtk_tree_path_free (path);//free memory
}

/*Autor: CST*/
/*fuegt einen breakpoint in die bp liste ein*/
void i8086event_addbp(GtkWidget *button, gpointer user_data)
{
        char *bpadrhex;
        unsigned int bpadr,seg;
        unsigned char isvalid=1,maxlen=7;
        
        bpadrhex=(char *)gtk_entry_get_text (GTK_ENTRY(bpadrtextbox));//neue breakpointadresse aus dem textfeld lesen
        isvalid=isValidHex(bpadrhex);//pruefen ob es sich um eine hexzahl handelt
	if(strstr(bpadrhex, ":")==NULL)
		maxlen=4;
        if(!isvalid || strlen(bpadrhex)>maxlen || !strlen(bpadrhex))//fehlerbehandlung
        {
               i8086guiMessage((GtkWidget *) user_data, "Please enter a valid hex-address (2:4 digits 0-f)!","", GTK_MESSAGE_ERROR);//fehlerbehandlung
        }
        else//wenn alles ok ist
        {
		if((strstr(bpadrhex, ":")==NULL))
		{
			sscanf((char *)bpadrhex,"%4x",&bpadr);//string in einen short umwandeln
			seg=0;
		}
		else
                	sscanf((char *)bpadrhex,"%02x:%4x",&seg,&bpadr);//string in einen short umwandeln
		bpadr+=(seg*0x10000);
                if(!addBp(bpadr))//wenn der neue breakpoint schon im array ist
                {
                       i8086guiMessage((GtkWidget *) user_data, "This breakpoint has already been set!","", GTK_MESSAGE_ERROR);//fehlerbehandlung
                }
                else//sonst
                {
                      updateTree();//tree aktualisieren
                }
         }               
  gtk_entry_set_text (GTK_ENTRY(bpadrtextbox),"");
  paint_disasm(disasmviewer, NULL, core, commands, 0);
}

/*Autor: CST*/                   
/*loescht einen Breakpoint aus der bp liste*/
void i8086event_delbp(GtkWidget *button, gpointer user_data)
{
unsigned int bpadr,seg;
sscanf(selectedRow,"%02x:%4x",&seg,&bpadr);//wandelt den hexwert der aktuellen in einen short
bpadr+=(seg*0x10000);
        if(delBp(bpadr))//wurde diese breakpoint erfolgreich aus dem array entfernt
                updateTree();//wird der tree neu gezeichnet
gtk_entry_set_text (GTK_ENTRY(bpadrtextbox),"");//das eingabe feld wird geloescht
paint_disasm(disasmviewer, NULL, core, commands, 0);//der disasm wird neu gemalt um evtl. neue breakpoints zu markieren
}   
