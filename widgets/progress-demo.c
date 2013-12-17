/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 * and Chris Mire (czb)
 *
 * MegaTunix pbar widget
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file widgets/progress-demo.c
  \brief MtxProgress demo application
  \author David Andruczyk
  */

#include <progress.h>

gboolean update_pbar_wrapper(gpointer );
gboolean update_pbar(gpointer );
gboolean close_demo(GtkWidget *,gpointer );

int main (int argc, char **argv)
{
	GtkWidget *window = NULL;
	GtkWidget *pbar = NULL;
	gint timeout = 0;

#if GLIB_MINOR_VERSION < 32
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	pbar = mtx_progress_bar_new ();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pbar),
			                        GTK_PROGRESS_BOTTOM_TO_TOP);

	gtk_container_add (GTK_CONTAINER (window), pbar);
	/*gtk_widget_realize(pbar);*/

	gtk_widget_show_all (window);

	timeout = g_timeout_add(30,(GSourceFunc)update_pbar_wrapper,(gpointer)pbar);

	g_signal_connect (window, "delete_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));
	g_signal_connect (window, "destroy_event",
			G_CALLBACK (close_demo), GINT_TO_POINTER(timeout));

	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	return 0;
}

gboolean update_pbar_wrapper(gpointer data)
{
	g_idle_add(update_pbar,data);
	return TRUE;
}

gboolean update_pbar(gpointer data)
{
	static gfloat lower = 0.0;
	static gfloat upper = 1.0;
	gfloat cur_val = 0.0;
	gfloat interval = 0.0;
	static gboolean rising = TRUE;

	GtkWidget * pbar = (GtkWidget *)data;
	interval = (upper-lower)/100.0;
	cur_val = mtx_progress_bar_get_fraction(MTX_PROGRESS_BAR (pbar));
	if (cur_val >= upper)
		rising = FALSE;
	if (cur_val <= lower)
		rising = TRUE;

	if (rising)
	{
		cur_val+=interval;
		if (cur_val > upper)
			cur_val = upper;
	}
	else
	{
		cur_val-=interval;
		if (cur_val < lower)
			cur_val = lower;
	}
	mtx_progress_bar_set_fraction (MTX_PROGRESS_BAR (pbar),cur_val);
	return FALSE;

}

gboolean close_demo(GtkWidget * widget, gpointer data)
{
	g_source_remove((guint)data);
	gtk_widget_destroy(widget);
	gtk_main_quit();
	return TRUE;
}
