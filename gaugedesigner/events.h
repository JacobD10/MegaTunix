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

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <defines.h>
#include <gauge.h>
#include <gtk/gtk.h>

typedef enum
{
	ADJ_LOW_UNIT_PARTNER,
	ADJ_HIGH_UNIT_PARTNER,
	ADJ_START_ANGLE_PARTNER,
	ADJ_SWEEP_ANGLE_PARTNER
}func;

typedef enum
{
	GENERAL_TAB = 0,
	TEXTBLOCK_TAB,
	TICKGROUP_TAB,
	WARNING_TAB,
	ALERT_TAB,
	POLYGON_TAB
}pages;

typedef enum
{
	IMPORT_XML = 0x1AA,
	EXPORT_XML
}StdButton;


/* Prototypes */
gboolean alter_polygon_data(GtkWidget *, gpointer );
gboolean animate_gauge(GtkWidget *, gpointer );
gboolean change_font(GtkWidget *, gpointer );
gboolean close_current_gauge(GtkWidget *, gpointer );
gboolean create_new_gauge(GtkWidget *, gpointer );
gboolean create_polygon_event(GtkWidget *, gpointer );
gboolean gauge_motion(GtkWidget *, GdkEventMotion *, gpointer );
gboolean gauge_button(GtkWidget *, GdkEventButton *, gpointer );
gboolean grab_coords_event(GtkWidget *, gpointer);
gboolean link_range_spinners(GtkWidget *, gpointer );
void update_attributes(void);
gboolean remove_polygon(GtkWidget *, gpointer );
void reset_onscreen_controls(void);
void reset_onscreen_polygons(void);
gboolean set_antialiased_mode(GtkWidget *, gpointer );
gboolean sweep_gauge(gpointer data);
gboolean toggle_skip_params(GtkWidget *, gpointer );
void update_onscreen_polygons(void);
/* Prototypes */

#endif
