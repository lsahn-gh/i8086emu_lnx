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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include "i8086error.h"

void i8086guiShowMessage(GtkWindow *parent, const char *msg, int mode)
{
  GtkWidget *dialog;
  
  dialog = gtk_message_dialog_new (parent,
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   mode,
                                   GTK_BUTTONS_CLOSE,
                                   msg);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

/* Gibt msg1 und msg2 durch \n getrennt auf dem */
/* Bildschirm aus. Beendet mit endwin() noch    */
/* offene curses-windows.                       */
void i8086guiErrorEx(GtkWidget *parent, const char *filename, int linenum, const char *msg1, const char *msg2)
{
  FILE *file;
  char lnum[16];
  char *message;
  
  message = (char*)calloc(strlen(msg1)+strlen("\n")+strlen(msg2), sizeof(char));
  strcat(message, msg1);
  strcat(message, "\n");
  strcat(message, msg2);
  
  file = fopen(i8086_LOG_FILE, "a");
  if (file!=NULL)
  {
    fputs("[Error] ", file);
    fputs(filename, file);
    fputs(": Line ", file);
    sprintf(lnum, "%d", linenum);
    fputs(lnum, file);
    fputs(": ", file);
    fputs(msg1, file);
    fputs(": ", file);
    fputs(msg2, file);
    fputc('\n', file);
    fclose(file);
  }
  
  i8086guiShowMessage(GTK_WINDOW(parent), message, GTK_MESSAGE_ERROR);
  free(message);
  if (parent!=NULL)
    g_signal_emit_by_name(parent, "delete-event");
}

