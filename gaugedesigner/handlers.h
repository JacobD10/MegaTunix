/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include <defines.h>
#include <gtk/gtk.h>

/* Prototypes */
gboolean about_menu_handler(GtkWidget *, gpointer );
gboolean alert_ranges_menu_handler(GtkWidget *, gpointer );
gboolean day_nite_handler(GtkWidget *, gpointer);
gboolean general_attributes_menu_handler(GtkWidget *, gpointer );
gboolean generic_spin_button_handler(GtkWidget *, gpointer );
gboolean polygon_menu_handler(GtkWidget *, gpointer );
gboolean quit_gaugedesigner(GtkWidget *, gpointer );
void reset_general_controls();
void reset_text_controls();
gboolean text_attributes_menu_handler(GtkWidget *, gpointer );
gboolean tg_spin_button_handler(GtkWidget *, gpointer );
gboolean tick_groups_menu_handler(GtkWidget *, gpointer );
void update_general_controls();
void update_text_controls();
gboolean warning_ranges_menu_handler(GtkWidget *, gpointer );
gboolean xml_button_handler(GtkWidget *, gpointer );
/* Prototypes */

#endif
