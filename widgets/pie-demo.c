/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * MegaTunix gauge widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*! 
  \file widgets/pie-demo.c
  \brief MtxPieGauge demo application
  \author David Andruczyk
  */

#include <piegauge.h>

gboolean update_gauge_wrapper(gpointer );
gboolean update_gauge(gpointer );
gboolean close_demo(GtkWidget *, gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *gauge = NULL;
	gint timeout = 0;

#if GLIB_MINOR_VERSION < 32
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gauge = mtx_pie_gauge_new ();
	gtk_container_add (GTK_CONTAINER (window), gauge);
	/*gtk_widget_realize(gauge);*/

	gtk_widget_show_all (window);

	/*mtx_pie_gauge_set_value(MTX_PIE_GAUGE(gauge), 0.0);*/

	/*mtx_gauge_face_set_attribute(MTX_PIE_GAUGE(gauge),LBOUND, 0.0);*/

	/*mtx_gauge_face_set_attribute(MTX_PIE_GAUGE(gauge),UBOUND, 8000.0);*/

	timeout = g_timeout_add(20,(GSourceFunc)update_gauge_wrapper,(gpointer)gauge);

	g_signal_connect (window, "delete_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));
	g_signal_connect (window, "destroy_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));

	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	return 0;
}

gboolean update_gauge_wrapper(gpointer data)
{
	g_idle_add(update_gauge,data);
	return TRUE;
}


gboolean update_gauge(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 100.0;
	gfloat cur_val = 0.0;
	gfloat interval = 0.0;
	static gboolean rising = TRUE;

	GtkWidget * gauge = (GtkWidget *)data;
	interval = (upper-lower)/100.0;
	/*mtx_gauge_face_get_attribute(MTX_PIE_GAUGE(gauge), LBOUND, &lower);*/

	/*mtx_gauge_face_get_attribute(MTX_PIE_GAUGE(gauge), UBOUND, &upper);*/

	cur_val = mtx_pie_gauge_get_value(MTX_PIE_GAUGE (gauge));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
		cur_val+=interval;
	else
		cur_val-=interval;

	mtx_pie_gauge_set_value (MTX_PIE_GAUGE (gauge),cur_val);
	return FALSE;
}

gboolean close_demo(GtkWidget *widget, gpointer data)
{
	g_source_remove((guint)data);
	gtk_widget_destroy(widget);
	gtk_main_quit();
	return TRUE;
}

