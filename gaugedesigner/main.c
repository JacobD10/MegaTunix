#include "events.h"
#include "../widgets/gauge.h"
#include <stdio.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <math.h>

int main (int argc, char ** argv )
{
	GtkWidget *window;
	GtkWidget *toptable;
	GladeXML *xml = NULL;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"destroy_event",
			G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(window),"delete_event",
			G_CALLBACK(gtk_main_quit),NULL);


	xml = glade_xml_new("gaugedesigner.glade", "toptable", NULL);
	glade_xml_signal_autoconnect(xml);
	toptable = glade_xml_get_widget(xml, "toptable");
	/* Bind the appropriate handlers */
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_ticks_spin")),"handler",GINT_TO_POINTER(MAJ_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_ticks_spin")),"handler",GINT_TO_POINTER(MIN_TICKS));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_len_spin")),"handler",GINT_TO_POINTER(MAJ_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_len_spin")),"handler",GINT_TO_POINTER(MIN_TICK_LEN));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"tick_inset_spin")),"handler",GINT_TO_POINTER(TICK_INSET));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_tail_spin")),"handler",GINT_TO_POINTER(NEEDLE_TAIL));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_width_spin")),"handler",GINT_TO_POINTER(NEEDLE_WIDTH));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"start_angle_spin")),"handler",GINT_TO_POINTER(START_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"stop_angle_spin")),"handler",GINT_TO_POINTER(STOP_ANGLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"lbound_spin")),"handler",GINT_TO_POINTER(LBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"ubound_spin")),"handler",GINT_TO_POINTER(UBOUND));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_string_entry")),"handler",GINT_TO_POINTER(NAME_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_string_entry")),"handler",GINT_TO_POINTER(UNITS_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"precision_spin")),"handler",GINT_TO_POINTER(PRECISION));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"antialiased_check")),"handler",GINT_TO_POINTER(AA));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_string_entry")),"handler",GINT_TO_POINTER(NAME_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_string_entry")),"handler",GINT_TO_POINTER(UNITS_STR));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_font_entry")),"handler",GINT_TO_POINTER(NAME_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_font_entry")),"handler",GINT_TO_POINTER(UNITS_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_entry")),"handler",GINT_TO_POINTER(VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_font_scale_spin")),"handler",GINT_TO_POINTER(NAME_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_font_scale_spin")),"handler",GINT_TO_POINTER(UNITS_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_font_scale_spin")),"handler",GINT_TO_POINTER(VALUE_SCALE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"name_color_button")),"handler",GINT_TO_POINTER(COL_NAME_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"units_color_button")),"handler",GINT_TO_POINTER(COL_UNIT_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"value_color_button")),"handler",GINT_TO_POINTER(COL_VALUE_FONT));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"background_color_button")),"handler",GINT_TO_POINTER(COL_BG));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"needle_color_button")),"handler",GINT_TO_POINTER(COL_NEEDLE));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"major_tick_color_button")),"handler",GINT_TO_POINTER(COL_MAJ_TICK));
	g_object_set_data(G_OBJECT(glade_xml_get_widget(xml,"minor_tick_color_button")),"handler",GINT_TO_POINTER(COL_MIN_TICK));

	gtk_container_add(GTK_CONTAINER(window),toptable);

	gtk_widget_show_all(window);

	gtk_main();
	return (0);
}