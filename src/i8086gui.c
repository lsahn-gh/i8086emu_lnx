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
/* Autor: FB                                        */
/* Version: 1.0                                     */
/*                                                  */
/*	Gui                                             */
/*                                                  */
/****************************************************/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtktext.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i8086gui.h"
#include "i8086proc.h"
#include "i8086error.h"
#include "i8086config.h"
#include "i8086datatrans.h"
#include "i8086messages.h"
#include "i8086config.h"
#include "i8086gui_emufuncs.h"
#include "i8086gui_util.h"
#include "i8086gui_paint.h"

G_CONST_RETURN gchar *filename;		//Datei Name
GtkWidget *file_selection_box; 		//Box für File-Open Dialog

//Autor: FB 
//Fenster Schliessen
void destroy(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  i8086guiStop(NULL, NULL);
  i8086guiCloseEmu();
  exit(0);
}

void onShow(GtkWidget *widget, gpointer user_data)
{
	if (i8086NameExistsConfig(CONFIG_FILE, "ROMFILE")!=0) /* ROM-Starten */
	  i8086gui_cpu_reset(NULL, NULL);
}


//Autor: FB
//erstellt für create _keyboard die Buttons
static GtkWidget *keybox (char *titel, gint  x, gint  y,unsigned int keypress)
{
  GtkWidget *fixed;
  GtkWidget *button;

  fixed = gtk_fixed_new ();//Fixed Box erzeugen

  gtk_widget_show (fixed);//Box anzeigen

  button = gtk_button_new_with_mnemonic (titel);//Button erzeugen
  gtk_widget_show (button);//Button zeigen
  gtk_fixed_put (GTK_FIXED (fixed), button, x, y);//Button auf unterlage legen, an eine bestimmte stelle
  gtk_widget_set_size_request (button, 24, 24);//button grösse festlegen
  g_signal_connect ((gpointer) button, "pressed", G_CALLBACK (i8086gui_keyboard_button_pressed),
  	(void*)  keypress);
  g_signal_connect ((gpointer) button, "released", G_CALLBACK (i8086gui_keyboard_button_released),NULL);
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (i8086gui_keyboard_button_clicked),
  	(void*)  keypress);

  return fixed;
}


//Autor: FB
//erstellt das Tastenfeld
static GtkWidget *create_keyboard ()
{
  GtkWidget *fixed;
  GtkWidget *button;

  fixed = gtk_fixed_new ();//Fixed Box erzeugen

  //erste Reihe Buttons
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_8\n", 0, 8, 0x06), 0, 0);//Button auf unterlage legen
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_9\n", 24, 8, 0x0E), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_A\n", 48, 8, 0x16), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_B\n", 72, 8, 0x1E), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_C\n", 96, 8, 0x26), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_D\n", 120, 8, 0x2E), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_E\n", 144, 8, 0x36), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_F\n", 168, 8, 0x3E), 0, 0);

  //Button zweite Reihe
  button = gtk_button_new_with_mnemonic ("_0\n");
  gtk_widget_show (button);
  gtk_fixed_put (GTK_FIXED (fixed), button, 0, 32);
  gtk_widget_set_size_request (button, 24, 24);
  g_signal_connect ((gpointer) button, "pressed", G_CALLBACK (i8086gui_keyboard_button_pressed),
  	(void*) ((0<<3)|5));
  g_signal_connect ((gpointer) button, "released", G_CALLBACK (i8086gui_keyboard_button_released), NULL);
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (i8086gui_keyboard_button_clicked), (void*)
  	((7<<3)|5));

  gtk_fixed_put (GTK_FIXED (fixed), keybox("_1\n", 24, 32, ((1<<3)|5)), 0, 0);//Button auf unterlage legen
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_2\n", 48, 32, ((2<<3)|5)), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_3\n", 72, 32, ((3<<3)|5)), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_4\n", 96, 32, ((4<<3)|5)), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_5\n", 120, 32,((5<<3)|5)), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_6\n", 144, 32, ((6<<3)|5)), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("_7\n", 168, 32, ((7<<3)|5)), 0, 0);

	//dritte Reihe Buttons
  gtk_fixed_put (GTK_FIXED (fixed), keybox("-\n", 0, 56, 0x03), 0, 0);//Button auf unterlage legen
  gtk_fixed_put (GTK_FIXED (fixed), keybox("+\n", 24, 56, 0x0B), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("O\n", 48, 56, 0x13), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("I\n", 72, 56, 0x1B), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("T\n", 96, 56, 0x23), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("S\n", 120, 56, 0x2B), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("G\n", 144, 56, 0x33), 0, 0);
  gtk_fixed_put (GTK_FIXED (fixed), keybox("E\n", 168, 56, 0x3B), 0, 0);

  return fixed;
}


//Autor: FB 
//Hilfsfunktion (erstellt die Radio Buttons in der Box)
static GtkWidget *create_switches ()
{  
	GtkWidget *frame;
  GtkWidget *fixed;
  GtkWidget *button;
  GtkWidget *label;
  char labelstr[]=" \n\0";
  char i;

  //Buttonframe und Rahmen erzeugen
  frame = gtk_frame_new ("");

  fixed = gtk_fixed_new ();
  gtk_widget_show (fixed);
  gtk_container_add (GTK_CONTAINER (frame), fixed);
  gtk_container_set_border_width (GTK_CONTAINER (fixed), 4);

  label = gtk_label_new (" Switches ");
  gtk_widget_show (label);
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  //Toggle Button erzeugen
  for (i=0;i<=7;i++)
  {
    labelstr[0]=(7-i)+48;
    button = gtk_toggle_button_new_with_label (labelstr);
    gtk_widget_show (button);
    gtk_fixed_put (GTK_FIXED (fixed), button, 18*i, 0);
    gtk_widget_set_size_request (button, 18, 23);
    g_signal_connect ((gpointer) button, "toggled",
                    G_CALLBACK (i8086gui_on_switch_toggled),
                    (gpointer) (7-i));
  }

  return frame;
}

//Autor: FB
// Extrahiert den Datei Namen aus dem File- Dialog
void store_filename()
{
  filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  i8086guiLoadBinFile(filename, i8086ReadHexConfig(CONFIG_FILE, "PROGSTARTADR", 0x0100));
}

//Autor: FB
//Datei Öffnen Dialog
void create_file_selection(void)
{
	//Erstellt den Datei öffnen dialog
  file_selection_box = gtk_file_selection_new("Please select a file.");

  //Linked den OK- Button zur Speichernden Fielname Funktion
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
                                  "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);

  //Zerstört den Dialog, wenn auf OK geklicked wurde
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button),
            "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) file_selection_box);

  //Zerstört den Dialog, wenn auf Cancel geklicked wurde
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->cancel_button),
                "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) file_selection_box);

  //Visualisiert dien File open Dialog
  gtk_widget_show (file_selection_box);
}




//Autor: FB
//Haupt Funktion
void do_appwindow (void)
{
  if (!window)
  {
    GtkAccelGroup *accel_group;
    GtkWidget *menuFile;
    GtkWidget *menuRun;
    GtkWidget *menuHelp;
    GtkWidget *menuOption;
    GtkWidget *menubar;
    GtkWidget *menu;
    GtkWidget *menuitem;
    GtkWidget *main_vbox;
    GtkWidget *root_vbox;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *hseparator;
    GtkWidget *scrolledwindow;
    GtkWidget *update_interval_scale;

    GtkWidget *handlebox;
    GtkWidget *handlebox2;
    GtkWidget *frame_leds;
    GtkWidget *frame_anz;
    GtkWidget *frame_disa;
    GtkWidget *frame_mem;
    GtkWidget *frame_reg;
    GtkWidget *frame_flag;
    GtkWidget *frame_keyb;

    GtkWidget *label;

    GtkWidget *toolbar;
    GtkWidget *button;
    GtkWidget *frame;

	  /*-- erstellt das Hauptfenster --*/
	  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	  /*-- erstellt eine vertikalle Box (Container) --*/
	  root_vbox = gtk_vbox_new(FALSE, 0);
	
	  /*-- fügt die Box in das Fenster ein --*/
	  gtk_container_add(GTK_CONTAINER(window), root_vbox);
	
	  /*-- window Titel --*/
	  gtk_window_set_title(GTK_WINDOW (window), "i8068 Emulator ["VERSION_NUMBER"]");

	  accel_group = gtk_accel_group_new ();
	  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
          
          #ifndef _WIN32
          gtk_window_set_icon_from_file   (GTK_WINDOW(window), PIXDIR"/i8086icon.png", NULL);
          #endif
	  /*-- erstellt die Handleboxes --*/
	  handlebox = gtk_handle_box_new();
	  handlebox2 = gtk_handle_box_new();

	  /*-- erstellt die Menubar --*/
  	menubar = gtk_menu_bar_new();

  	//Fenster schliessen
    g_signal_connect (window, "delete-event", G_CALLBACK (destroy), &window);
    /* Fenster fertig initialisiert */
    g_signal_connect (window, "show", G_CALLBACK (onShow), &window);


/****************   Menü   *******************/

	  // *** File Menü ***
	  menuFile = gtk_menu_item_new_with_mnemonic ("F_ile");
	  gtk_menu_bar_append (GTK_MENU_BAR(menubar), menuFile);
	  gtk_widget_show(menuFile);
	  //Sub File Menü
	  menu = gtk_menu_new();
	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuFile), menu);
		//Open
	  menuitem = gtk_image_menu_item_new_with_label ("Open");
	  gtk_menu_append(GTK_MENU(menu), menuitem);
	  gtk_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC (create_file_selection), NULL);
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	  gtk_widget_show (menuitem);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU));
  	//Exit
	  menuitem = gtk_image_menu_item_new_with_label ("Quit");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	gtk_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC (destroy), NULL);
  	gtk_widget_show (menuitem);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_MENU));
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

   	// *** Run Menü ***
	  menuRun = gtk_menu_item_new_with_mnemonic ("_Run");
	  gtk_menu_bar_append (GTK_MENU_BAR(menubar), menuRun);
	  gtk_widget_show(menuRun);
	  //Sub run Menü
	  menu = gtk_menu_new();
	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuRun), menu);
	  //Next
	  tracemenuitem = gtk_image_menu_item_new_with_label ("Trace");
	  gtk_menu_append(GTK_MENU(menu), tracemenuitem);
	  gtk_widget_show (tracemenuitem);
	  gtk_signal_connect(GTK_OBJECT (tracemenuitem), "activate", GTK_SIGNAL_FUNC (i8086guiNextCommand), NULL);
	  gtk_widget_add_accelerator (tracemenuitem, "activate", accel_group, GDK_F7, 0, GTK_ACCEL_VISIBLE);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tracemenuitem), gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_MENU));
	  //StepOver
	  stepovermenuitem = gtk_image_menu_item_new_with_label ("Step Over");
	  gtk_menu_append(GTK_MENU(menu), stepovermenuitem);
	  gtk_widget_show (stepovermenuitem);
	  gtk_signal_connect(GTK_OBJECT (stepovermenuitem), "activate", GTK_SIGNAL_FUNC (i8086guiStepOver), NULL);
	  gtk_widget_add_accelerator (stepovermenuitem, "activate", accel_group, GDK_F8, 0, GTK_ACCEL_VISIBLE);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stepovermenuitem), gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU));
	  //Run
	  runmenuitem = gtk_image_menu_item_new_with_label ("Run");
	  gtk_menu_append(GTK_MENU(menu), runmenuitem);
	  gtk_widget_show (runmenuitem);
	  gtk_signal_connect(GTK_OBJECT (runmenuitem), "activate", GTK_SIGNAL_FUNC (i8086guiRun), NULL);
	  gtk_widget_add_accelerator (runmenuitem, "activate", accel_group, GDK_F9, 0, GTK_ACCEL_VISIBLE);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (runmenuitem), gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU));
	  //Stop
	  menuitem = gtk_image_menu_item_new_with_label ("Stop");
	  gtk_menu_append(GTK_MENU(menu), menuitem);
	  gtk_widget_show (menuitem);
	  gtk_signal_connect(GTK_OBJECT (menuitem), "activate", GTK_SIGNAL_FUNC (i8086guiStop), NULL);
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_Escape, 0, GTK_ACCEL_VISIBLE);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-stop", GTK_ICON_SIZE_MENU));

	  // *** Option Menü ***
	  menuOption = gtk_menu_item_new_with_mnemonic ("_Options");
	  gtk_menu_bar_append (GTK_MENU_BAR(menubar), menuOption);
	  gtk_widget_show(menuOption);
	  //Sub Option Menü
	  menu = gtk_menu_new();
	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuOption), menu);
	  //Register Inhalt ändern
  	menuitem = gtk_image_menu_item_new_with_label ("Change Register");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	gtk_widget_show (menuitem);
  	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-find-and-replace", GTK_ICON_SIZE_MENU));
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(GTK_OBJECT (menuitem), "activate", G_CALLBACK (changeMemReg),(void*) 0);
  	//Speicher Addressen Inhalt ändern
  	menuitem = gtk_image_menu_item_new_with_label ("Change Mem");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	gtk_widget_show (menuitem);
  	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-find-and-replace", GTK_ICON_SIZE_MENU));
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_m, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(GTK_OBJECT (menuitem), "activate", G_CALLBACK (changeMemReg),(void*) 1);

    //Breakpoint menuoption
  	menuitem = gtk_image_menu_item_new_with_label ("Set/Del Breakpoint");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	gtk_widget_show (menuitem);
    g_signal_connect(GTK_OBJECT (menuitem), "activate", G_CALLBACK (setBreakpoint),menuitem);
  	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-no", GTK_ICON_SIZE_MENU));
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);


	   // *** Help/About Menü ***
	  menuHelp = gtk_menu_item_new_with_mnemonic ("Abo_ut");
	  gtk_menu_bar_append (GTK_MENU_BAR(menubar), menuHelp);
	  gtk_widget_show(menuHelp);
	  //Sub Help Menü
	  menu = gtk_menu_new();
	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuHelp), menu);
	  /*/Help
  	menuitem = gtk_image_menu_item_new_with_label ("Help");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	g_signal_connect(GTK_OBJECT (menuitem), "activate", G_CALLBACK (help_win),(void*) 1);
  	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-help", GTK_ICON_SIZE_MENU));
	  gtk_widget_add_accelerator (menuitem, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
  	gtk_widget_show (menuitem);*/
  	//About 
  	menuitem = gtk_image_menu_item_new_with_label ("About");
  	gtk_menu_append(GTK_MENU(menu), menuitem);
  	gtk_widget_show (menuitem);
  	g_signal_connect(GTK_OBJECT (menuitem), "activate", G_CALLBACK (about_win),(void*) 1);
	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU));
  	
  	//Menü an die Handlebox anbinden
	  gtk_container_add(GTK_CONTAINER(handlebox), menubar);
	  //Handlebox auf Vbox stellen
  	gtk_box_pack_start(GTK_BOX(root_vbox), handlebox, FALSE, TRUE, 0);

  	
				  	 			// ***************  Toolbar  ********+********** //
      
	  //Toolbar erstellen
    toolbar = gtk_toolbar_new ();
    gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar),
                                GTK_STOCK_OPEN,
                                "Open File",
                                NULL,
                                G_CALLBACK (create_file_selection),
                                window, -1);  
      
    tracebutton = gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar),
                                GTK_STOCK_JUMP_TO,
                                "Trace Into",
                                NULL,
                                G_CALLBACK(i8086guiNextCommand),
                                window, -1);

    stepoverbutton = gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar),
                                GTK_STOCK_GO_FORWARD,
                                "Step Over",
                                NULL,
                                G_CALLBACK(i8086guiStepOver),
                                window, -1);

    runbutton = gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar),
                                GTK_STOCK_EXECUTE,
                                "Run Programm",
                                NULL,
                                G_CALLBACK(i8086guiRun),
                                window, -1);

    gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar),
                                GTK_STOCK_STOP,
                                "Stop a running Programm",
                                NULL,
                                G_CALLBACK (i8086guiStop),
                                window, -1);
    //Abstand
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));    //Abstand

    update_interval_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (50, 1, 50, 5, 10, 0)));
    gtk_scale_set_draw_value (GTK_SCALE (update_interval_scale), FALSE);
    gtk_scale_set_digits (GTK_SCALE (update_interval_scale), 0);
    gtk_range_set_inverted (GTK_RANGE (update_interval_scale), TRUE);
    gtk_widget_set_size_request (update_interval_scale, 120, 20);
    g_signal_connect ((gpointer) update_interval_scale, "value_changed",
                    G_CALLBACK (i8086gui_adjust_update_interval),
                    NULL);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), " Update Interval ");
    gtk_toolbar_append_widget(GTK_TOOLBAR (toolbar),label,NULL,NULL);
    gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), update_interval_scale, NULL, NULL);

    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);


	  gtk_container_add(GTK_CONTAINER(handlebox2), toolbar);
	  //Handlebox auf Vbox stellen
  	gtk_box_pack_start(GTK_BOX(root_vbox), handlebox2, FALSE, TRUE, 0);


  	    		// ***************  Fensterboxes	****************** //

    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start (GTK_BOX (root_vbox), main_vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 7);

  	// *** Obere Fenster *** //


    //Fenster Disassembler
    hbox = gtk_hpaned_new ();
    gtk_box_pack_start (GTK_BOX (main_vbox), hbox, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_paned_pack1 (GTK_PANED (hbox), vbox, TRUE, TRUE);

    //Fensterlabels der oberen Fenster und memviewer change
    label= gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<b>Disassembler</b>");
    gtk_widget_set_size_request (label, 1, 22);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    frame_disa = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_disa), GTK_SHADOW_NONE);
    gtk_box_pack_start (GTK_BOX (vbox), frame_disa, TRUE, TRUE, 0);

    //dis-assemmbler
    disasmviewer =(GtkWidget *) gtk_text_new(NULL, NULL);
    gtk_widget_set_size_request (frame_disa, 150, 130);
    gtk_text_insert(GTK_TEXT(disasmviewer), NULL, NULL, NULL, "", -1);
    gtk_container_add (GTK_CONTAINER (frame_disa), disasmviewer);
    gtk_text_set_line_wrap(GTK_TEXT(disasmviewer), FALSE);
    gtk_text_set_editable(GTK_TEXT(disasmviewer), FALSE);
    gtk_text_set_word_wrap(GTK_TEXT(disasmviewer), FALSE);
    g_signal_connect ((gpointer) disasmviewer, "size_allocate", G_CALLBACK (on_disasmviewer_size_allocate), NULL);
    gtk_widget_show(disasmviewer);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_paned_pack2 (GTK_PANED (hbox), vbox, TRUE, TRUE);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<b>Memory</b>");
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

   	//Speicher Anzeige Aenderungs-Feld
 	  memTextField = gtk_entry_new ();
 	  gtk_box_pack_start (GTK_BOX (hbox), memTextField, FALSE, FALSE, 0);
 	  gtk_entry_set_max_length (GTK_ENTRY (memTextField), 9);
 	  gtk_entry_set_width_chars (GTK_ENTRY (memTextField), 10);
 	  gtk_entry_set_text (GTK_ENTRY (memTextField), ("0000:0000"));
 	  g_signal_connect((gpointer) memTextField, "activate",
  		                G_CALLBACK (on_memtextfield_change_value),
   			              NULL);

   	memSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0x10FFEF, 16, 176, 16)), 16, 0);
   	gtk_box_pack_start (GTK_BOX (hbox), memSpinButton, FALSE, FALSE, 0);
   	gtk_widget_set_size_request (memSpinButton, 14, 1);
   	g_signal_connect((gpointer) memSpinButton, "value-changed",
   	             	    G_CALLBACK (on_memspin_change_value),
    	            	  NULL);

    frame_mem = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_mem),  GTK_SHADOW_NONE);
    gtk_box_pack_start (GTK_BOX (vbox), frame_mem, TRUE, TRUE, 0);
		//Scrollbar
    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_container_add (GTK_CONTAINER (frame_mem), scrolledwindow);
  
    //Fenster	MEM-Viewer
    memviewer =(GtkWidget *) gtk_text_new(NULL, NULL);
    gtk_widget_set_size_request (frame_mem, 350, 130);
    gtk_text_set_line_wrap(GTK_TEXT(memviewer), FALSE);
    gtk_text_set_editable(GTK_TEXT(memviewer), FALSE);
    gtk_text_set_word_wrap(GTK_TEXT(memviewer), FALSE);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), memviewer);
    g_signal_connect ((gpointer) memviewer, "size_allocate", G_CALLBACK (on_memviewer_size_allocate), NULL);
    gtk_widget_show(memviewer);

    //Trennlinie
    hseparator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (main_vbox), hseparator, FALSE, FALSE, 6);

    // *** Untere Fenster *** //

    hbox = gtk_hbox_new (FALSE, 0);//Horizontale Box2
    gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);

    //Fenster Display/Keyboard
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);

    frame_keyb = gtk_frame_new ("");
    label = gtk_label_new (" Display/Keyboard Unit ");
    gtk_frame_set_label_widget (GTK_FRAME (frame_keyb), label);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (vbox),frame_keyb, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_keyb), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

    //Display
    frame_anz = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_anz),  GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (vbox), frame_anz, TRUE, FALSE, 0);
    anz = gtk_drawing_area_new ();
    gtk_widget_set_size_request (frame_anz, 140, 36);
    gtk_container_add (GTK_CONTAINER (frame_anz), anz);
    g_signal_connect (anz, "expose_event", G_CALLBACK (paint_display), NULL);

    //Keyboard
    gtk_box_pack_start (GTK_BOX (vbox),create_keyboard(), FALSE, FALSE, 0);//Tasten in Keyboard Frame

    //Zwischenraum
    label = gtk_label_new ("");
		gtk_container_add (GTK_CONTAINER (vbox), label);
		gtk_widget_set_size_request (label, 140, 6);	 //Grösse zu weisen
 
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);
    //Fenster	Led's
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), " Led's ");
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

    frame_leds = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_leds),  GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (vbox), frame_leds, FALSE, FALSE, 0);
    gtk_frame_set_label_widget (GTK_FRAME (frame_leds), label);

    leds = gtk_drawing_area_new (); 										 //Leds als Zeichen-Fenster erstellen
    gtk_widget_set_size_request (frame_leds, 140, 40);	 //Grösse zu weisen

    gtk_container_add (GTK_CONTAINER (frame_leds), leds);//In Container Frame Leds packen
    g_signal_connect (leds, "expose_event", G_CALLBACK (paint_leds), NULL);

    //Fenster Switches
    gtk_box_pack_start (GTK_BOX (vbox),create_switches(),	FALSE, FALSE, 0);

    //Reset cpu button
    button = gtk_button_new_with_mnemonic ("CPU-Reset");
    g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (i8086gui_cpu_reset), NULL);
    gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new(NULL), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

    //Zwischenraum
    gtk_box_set_spacing (GTK_BOX (hbox), 8);
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, 0);

    //Fenster Register
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), " Register ");
    frame_reg = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_reg),  GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (vbox), frame_reg, TRUE, TRUE, 0);
    gtk_frame_set_label_widget (GTK_FRAME (frame_reg), label);

    reglabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (reglabel),"");
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_add (GTK_CONTAINER (frame_reg), frame);
    gtk_container_add (GTK_CONTAINER (frame), reglabel);
    i8086guiPaintAllRegister(reglabel);//schreibt die Regs

    // Fenster6 Flags
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), " Flags ");

    frame_flag= gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_flag),  GTK_SHADOW_IN);
    gtk_box_pack_start (GTK_BOX (vbox), frame_flag, TRUE, TRUE, 0);
    gtk_frame_set_label_widget (GTK_FRAME (frame_flag), label);

    flaglabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (flaglabel),"");
    gtk_container_add (GTK_CONTAINER (frame_flag), flaglabel);
    i8086guiPaintAllFlags(flaglabel);//schreibt die FLAGS

    statusbar = gtk_statusbar_new ();
    gtk_widget_show (statusbar);
    gtk_box_pack_start (GTK_BOX (root_vbox), statusbar, FALSE, FALSE, 0);

	  /*-- zeigt alle widgets --*/
	  gtk_widget_show_all (window);
	}
	  
	i8086guiPaintAll();	//mallt Register,Flags,....
  showStatusbarMessage("Ready");

  //Gtk Schleife
  gtk_main ();
}


int main( int   argc,
          char *argv[] )
{

  if((argc == 2) && (strcmp(argv[1],"--version")==0))
  {
                    printf("i8086emu "VERSION_NUMBER"\n"
                    "Copyright (C) 2004 JMH, RD, FB, CST\n"
                    "i8086emu comes with NO WARRANTY,\n"
                    "to the extent permitted by law.\n"
                    "You may redistribute copies of i8086emu\n"
                    "under the terms of the GNU General Public License.\n");
                    exit(1);
    }
          
  if (i8086ReadBoolConfig(CONFIG_FILE, "USEXFT", 1)==0)
    setenv("GDK_USE_XFT", "0", 1); /* Verhindert error bei fehlendem Fontserver */

  /* Gtk Initialisieren */
  gtk_init(&argc, &argv);

  /* Emu initialisieren */  i8086guiInitEmu();


  /* Fenster Funktion */
  do_appwindow();
      
  return(0);
}
