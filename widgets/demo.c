/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * Megasquirt gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <gauge.h>
#include <math.h>

gboolean update_gauge(gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *gauge;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_gauge_face_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);
	mtx_gauge_face_set_bounds (MTX_GAUGE_FACE (gauge), 0.0, 8000.0);
	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge), 0.0);
	/* the nest two are the same thing in different units.  NOTE the
	 * reverse sign on the deg version,  because of the fact that the
	 * rendering routiens do things ass backwards
	 */
	mtx_gauge_face_set_span_rad (MTX_GAUGE_FACE (gauge),0 , 1.5 * M_PI);
//	mtx_gauge_face_set_span_deg (MTX_GAUGE_FACE (gauge),0 , -270);
	//mtx_gauge_face_set_span_rad (MTX_GAUGE_FACE (gauge),  -1.25*M_PI,  .25*M_PI);
	//mtx_gauge_face_set_span_rad (MTX_GAUGE_FACE (gauge),  -1.0*M_PI,  .25*M_PI);
	mtx_gauge_face_set_antialias (MTX_GAUGE_FACE (gauge), TRUE);
	GdkColor color = { 0, 50000,50000,0};
	mtx_gauge_face_set_color_range(MTX_GAUGE_FACE(gauge), 6000, 7000, color, 0.06, 0.785);;
	color.red = 50000;
	color.green = 0;
	mtx_gauge_face_set_color_range(MTX_GAUGE_FACE(gauge), 7000, 8000, color, 0.06, 0.785);;
	mtx_gauge_face_set_name_str (MTX_GAUGE_FACE (gauge), "RPM");
	mtx_gauge_face_set_units_str (MTX_GAUGE_FACE (gauge), "");
	mtx_gauge_face_set_major_ticks (MTX_GAUGE_FACE (gauge), 9);
	mtx_gauge_face_set_minor_ticks (MTX_GAUGE_FACE (gauge), 3);
	mtx_gauge_face_set_precision (MTX_GAUGE_FACE (gauge), 0);
	gtk_timeout_add(20,(GtkFunction)update_gauge,(gpointer)gauge);

	testload(gauge);
	output_xml(gauge);
	//gtk_widget_set_size_request(window,500,500);


	gtk_main ();
	return 0;
}

gboolean update_gauge(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 0.0;
	gfloat cur_val = 0.0;
	gfloat interval = 0.0;
	static gboolean rising = TRUE;

	GtkWidget * gauge = data;
	interval = (upper-lower)/100.0;
	mtx_gauge_face_get_bounds(MTX_GAUGE_FACE (gauge),&lower,&upper);
	cur_val = mtx_gauge_face_get_value(MTX_GAUGE_FACE (gauge));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
		cur_val+=interval;
	else
		cur_val-=interval;

	mtx_gauge_face_set_value (MTX_GAUGE_FACE (gauge),cur_val);

	return TRUE;
}
