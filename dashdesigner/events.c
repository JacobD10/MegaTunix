/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Electronic Fuel Injection tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */



#include <getfiles.h>
#include <events.h>
#include <gauge.h>
#include <loadsave.h>
#include <rtv_parser.h>
#include <xml.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795 
#endif

gboolean changed = FALSE;
static gboolean grabbed = FALSE;
static gboolean resizing = FALSE;
static gboolean moving = FALSE;
static gint corner = -1;
static GtkBuilder *properties = NULL;
static GtkBuilder *previews = NULL;
static GtkWidget *grabbed_widget = NULL;
static GdkColor white = { 0, 65535, 65535, 65535};
static GdkColor home_color = {0, 62000, 59000, 65535};
static GdkColor title_color = {0, 58000, 58000, 58000};
extern GtkBuilder *toplevel;
static struct 
{
	gint child_x_origin;
	gint child_y_origin;
	gint child_width;
	gint child_height;
	gint rel_grab_x;
	gint rel_grab_y;
}tt = {0,0,0,0,0,0};

typedef enum 
{
	LL = 10,
	LR,
	UL,
	UR
}CornerType;

G_MODULE_EXPORT gboolean dashdesigner_about(GtkWidget * UNUSED(widget), gpointer UNUSED(data))
{
	extern GtkWidget *main_window;
#if GTK_MINOR_VERSION >= 8
	if (gtk_minor_version >= 8)
	{
		const gchar *authors[] = {"David Andruczyk",NULL};
		const gchar *artists[] = {"Dale Anderson",NULL};
		gtk_show_about_dialog(GTK_WINDOW(main_window),
				"name","MegaTunix Dashboard Designer",
				"version",VERSION,
				"copyright","David J. Andruczyk(2012)",
				"comments","Dashboard Designer is a tool to design custom Dash gauge layouts for the MegaTunix Tuning Software",
				"license","GNU GPL v2",
				"website","http://megatunix.sourceforge.net",
				"authors",authors,
				"artists",artists,
				"documenters",authors,
				NULL);
	}
#endif
	return TRUE;
}


G_MODULE_EXPORT gboolean create_preview_list(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{

	static gboolean created = FALSE;
	static gboolean prop_created = FALSE;
	GtkWidget * gauge = NULL;
	GtkWidget * table = NULL;
	GtkWidget * window = NULL;
	GtkWidget * notebook = NULL;
	GtkWidget * label = NULL;
	GtkWidget * swin = NULL;
	GtkWidget * ebox = NULL;
	GtkWidget * vbox = NULL;
	GtkWidget * sep = NULL;
	GArray *classes = NULL;
	gchar ** vector = NULL;
	gchar ** files = NULL;
	gchar * filename = NULL;
	gchar * tmpbuf = NULL;
	gchar * path = NULL;
	GDir *dir = NULL;
	gchar *d_name = NULL;
	gint t_num = -1;
	GList *dir_list = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	guint i = 0;
	GError *error = NULL;
	gchar *file = NULL;
	if (created)
	{
		window = GTK_WIDGET(gtk_builder_get_object(previews,"preview_window"));
		if (GTK_IS_WIDGET(window))
			gtk_widget_show_all(window);
	}
	else
	{
		file = g_build_filename(DASHDESIGNER_GLADE_DIR,"preview.ui",NULL);
		filename = get_file(NULL,file,NULL);
		g_free(file);
		if (filename)
		{
			previews = gtk_builder_new();
			if(!gtk_builder_add_from_file(previews,filename,&error))
			{
				g_warning ("Couldn't load builder file: %s", error->message);
				g_error_free(error);
			}
			gtk_builder_connect_signals(previews,NULL);
		}
	}

	if (prop_created)
	{
		window = GTK_WIDGET(gtk_builder_get_object(properties,"property_editor_window"));
		if (GTK_IS_WIDGET(window))
			gtk_widget_show_all(window);
	}
	else
	{
		file =  g_build_filename(DASHDESIGNER_GLADE_DIR,"propeditor.ui",NULL);
		filename = get_file(NULL,file,NULL);
		g_free(file);
		if (filename)
		{
			properties = gtk_builder_new();
			if(!gtk_builder_add_from_file(properties,filename,&error))
			{
				g_warning ("Couldn't load builder file: %s", error->message);
				g_error_free(error);
			}
			gtk_builder_connect_signals(properties,NULL);
		}
	}
	if (created && prop_created)
		return TRUE;


	notebook = GTK_WIDGET(gtk_builder_get_object(previews,"preview_notebook"));
	if (!notebook)
	{
		printf("critical error, notbook not found, EXITING!\n");
		exit (-1);
	}
	/* get rid of default empty page from ui... */


	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),0);
#ifdef __WIN32__
	path = g_build_path(PSEP,HOME(),"dist",GAUGES_DATA_DIR,NULL);
#else
	path = g_build_path(PSEP,DATA_DIR,GAUGES_DATA_DIR,NULL);
#endif
	files = get_files(NULL,GAUGES_DATA_DIR,"xml",&classes);
	i = 0;
	if (files)
	{
		while (files[i])
		{
			if (g_array_index(classes,FileClass,i) == PERSONAL)
				p_list = g_list_append(p_list,g_strdup(files[i]));
			if (g_array_index(classes,FileClass,i) == SYSTEM)
				s_list = g_list_append(s_list,g_strdup(files[i]));
			i++;
		}
		p_list = g_list_sort(p_list,list_sort);
		s_list = g_list_sort(s_list,list_sort);
		gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook),TRUE);

		ebox = gtk_event_box_new();
		g_signal_connect(G_OBJECT(ebox),
				"button_press_event",
				G_CALLBACK(gauge_choice_button_event),
				NULL);
		swin = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
				GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swin),
				ebox);
		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(ebox),vbox);
		table = gtk_table_new(1,1,FALSE);
		OBJ_SET((ebox),"table",table);
		gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);
		label = gtk_label_new("Unsorted");
		t_num = gtk_notebook_append_page(GTK_NOTEBOOK(notebook),swin,label);
		gtk_widget_show_all(notebook);
		for (i=0;i<g_list_length(p_list);i++)
		{
			vbox = gtk_vbox_new(FALSE,1);
			gtk_table_attach_defaults(GTK_TABLE(table),vbox,0,1,i,i+1);
			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,FALSE,0);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&title_color);
			vector = g_strsplit((gchar *)g_list_nth_data(p_list,i),PSEP,-1);
			tmpbuf = g_strdup_printf("%s",vector[g_strv_length(vector)-1]);
			g_strfreev(vector);
			label = gtk_label_new(tmpbuf);
			g_free(tmpbuf);
			gtk_container_add(GTK_CONTAINER(ebox),label);

			ebox = gtk_event_box_new();
                        gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);

			gauge = mtx_gauge_face_new();
			OBJ_SET(vbox,"gauge",gauge);
			gtk_container_add(GTK_CONTAINER(ebox),gauge);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),(gchar *)g_list_nth_data(p_list,i));
			gtk_widget_set_usize(GTK_WIDGET(gauge),200,200);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&home_color);
			sep = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,FALSE,0);
			gtk_widget_show(gauge);
		}
		for (i=0;i<g_list_length(s_list);i++)
		{
			vbox = gtk_vbox_new(FALSE,1);
                        gtk_table_attach_defaults(GTK_TABLE(table),vbox,0,1,i,i+1);
			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,FALSE,0);
                        gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&title_color);
                        vector = g_strsplit((gchar *)g_list_nth_data(s_list,i),PSEP,-1);
                        tmpbuf = g_strdup_printf("%s",vector[g_strv_length(vector)-1]);
                        g_strfreev(vector);
                        label = gtk_label_new(tmpbuf);
                        g_free(tmpbuf);
                        gtk_container_add(GTK_CONTAINER(ebox),label);

                        ebox = gtk_event_box_new();
                        gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);

			gauge = mtx_gauge_face_new();
			OBJ_SET(vbox,"gauge",gauge);
			gtk_container_add(GTK_CONTAINER(ebox),gauge);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),(gchar *)g_list_nth_data(s_list,i));
			gtk_widget_set_usize(GTK_WIDGET(gauge),200,200);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&white);
			sep = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,FALSE,0);
			gtk_widget_show(gauge);
		}

		g_array_free(classes,TRUE);
		classes = NULL;
		g_strfreev(files);
		g_list_foreach(p_list,free_element,NULL);
		g_list_foreach(s_list,free_element,NULL);
		g_list_free(p_list);
		g_list_free(s_list);
		p_list = NULL;
		s_list = NULL;
	}
#ifdef __WIN32__
	path = g_build_path(PSEP,HOME(),"dist",GAUGES_DATA_DIR,NULL);
#else
	path = g_build_path(PSEP,DATA_DIR,GAUGES_DATA_DIR,NULL);
#endif
	dir = g_dir_open(path,0,NULL);
	if (dir)
	{
		d_name = (gchar *)g_dir_read_name(dir);
		while (d_name != NULL)
		{
			dir_list = g_list_prepend(dir_list,g_strdup(d_name)); /* Store this dirname for next run thru */
			d_name = (gchar *)g_dir_read_name(dir);
		}
		g_dir_close(dir);
	}
	g_free(path);
	path = g_build_path(PSEP,HOME(),".MegaTunix",GAUGES_DATA_DIR,NULL);
	dir = g_dir_open(path,0,NULL);
	if (dir)
	{
		d_name = (gchar *)g_dir_read_name(dir);
		while ((d_name != NULL))
		{
			if (g_list_find_custom(dir_list,d_name,(GCompareFunc)g_strcmp0) != NULL)	/* we already got this dir*/
			{
				d_name = (gchar *)g_dir_read_name(dir);
				continue;
			}
			d_name = (gchar *)g_dir_read_name(dir);

		}
		g_dir_close(dir);
	}
	g_free(path);
	dir_list = g_list_sort(dir_list,list_sort);
	g_list_foreach(dir_list,scan_for_gauges,notebook);

	g_list_foreach(dir_list,(GFunc)g_free,NULL);
	g_list_free(dir_list);

	created = TRUE;
	prop_created = TRUE;
	return TRUE;
}


void scan_for_gauges(gpointer data, gpointer user_data)
{
	gchar * d_name = NULL;
	guint i = 0;
	gint t_num = -1;
	GtkWidget * notebook = NULL;
	GtkWidget * gauge = NULL;
	GtkWidget * table = NULL;
	GtkWidget * window = NULL;
	GtkWidget * label = NULL;
	GtkWidget * swin = NULL;
	GtkWidget * ebox = NULL;
	GtkWidget * vbox = NULL;
	GtkWidget * sep = NULL;
	GArray *classes = NULL;
	gchar * filename = NULL;
	gchar * path = NULL;
	gchar ** files = NULL;
	gchar ** vector = NULL;
	gchar *tmpbuf = NULL;
	GList *p_list = NULL;
	GList *s_list = NULL;
	gchar *pathstub = NULL;


	d_name = (gchar *)data;
	notebook = (GtkWidget *)user_data;

	g_return_if_fail(d_name);
	g_return_if_fail(notebook);
	pathstub = g_strconcat(GAUGES_DATA_DIR,PSEP,d_name,NULL);
	files = get_files(NULL,pathstub,"xml",&classes);
	g_free(pathstub);
	if (files)
	{
		i = 0;
		while (files[i])
		{
			if (g_array_index(classes,FileClass,i) == PERSONAL)
				p_list = g_list_append(p_list,g_strdup(files[i]));
			if (g_array_index(classes,FileClass,i) == SYSTEM)
				s_list = g_list_append(s_list,g_strdup(files[i]));
			i++;
		}
		p_list = g_list_sort(p_list,list_sort);
		s_list = g_list_sort(s_list,list_sort);

		ebox = gtk_event_box_new();
		g_signal_connect(G_OBJECT(ebox),
				"button_press_event",
				G_CALLBACK(gauge_choice_button_event),
				NULL);
		swin = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(
				GTK_SCROLLED_WINDOW(swin),
				GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_add_with_viewport(
				GTK_SCROLLED_WINDOW(swin),
				ebox);
		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(ebox),vbox);
		table = gtk_table_new(1,1,FALSE);
		gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);
		OBJ_SET((ebox),"table",table);
		label = gtk_label_new(d_name);
		t_num = gtk_notebook_append_page(
				GTK_NOTEBOOK(notebook),
				swin,label);
		i = 0;
		gtk_widget_show_all(ebox);
		for (i=0;i<g_list_length(p_list);i++)
		{
			vbox = gtk_vbox_new(FALSE,1);
			gtk_table_attach_defaults(GTK_TABLE(table),vbox,0,1,i,i+1);
			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,FALSE,0);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&title_color);
			vector = g_strsplit((gchar *)g_list_nth_data(s_list,i),PSEP,-1);
			tmpbuf = g_strdup_printf("%s",vector[g_strv_length(vector)-1]);
			g_strfreev(vector);
			label = gtk_label_new(tmpbuf);
			g_free(tmpbuf);
			gtk_container_add(GTK_CONTAINER(ebox),label);

			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
			gauge = mtx_gauge_face_new();
			OBJ_SET(vbox,"gauge",gauge);
			gtk_container_add(GTK_CONTAINER(ebox),gauge);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge), (gchar *)g_list_nth_data(p_list,i));
			gtk_widget_set_usize(GTK_WIDGET(gauge),200,200);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&home_color);
			sep = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,FALSE,0);
			gtk_widget_show(gauge);
		}
		for (i=0;i<g_list_length(s_list);i++)
		{
			vbox = gtk_vbox_new(FALSE,1);
			gtk_table_attach_defaults(GTK_TABLE(table),vbox,0,1,i,i+1);
			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,FALSE,FALSE,0);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&title_color);
			vector = g_strsplit((gchar *)g_list_nth_data(s_list,i),PSEP,-1);
			tmpbuf = g_strdup_printf("%s",vector[g_strv_length(vector)-1]);
			g_strfreev(vector);
			label = gtk_label_new(tmpbuf);
			g_free(tmpbuf);
			gtk_container_add(GTK_CONTAINER(ebox),label);

			ebox = gtk_event_box_new();
			gtk_box_pack_start(GTK_BOX(vbox),ebox,TRUE,TRUE,0);
			gauge = mtx_gauge_face_new();
			OBJ_SET(vbox,"gauge",gauge);
			gtk_container_add(GTK_CONTAINER(ebox),gauge);
			mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge), (gchar *)g_list_nth_data(s_list,i));
			gtk_widget_set_usize(GTK_WIDGET(gauge),200,200);
			gtk_widget_modify_bg(GTK_WIDGET(ebox),GTK_STATE_NORMAL,&white);
			sep = gtk_hseparator_new();
			gtk_box_pack_start(GTK_BOX(vbox),sep,TRUE,FALSE,0);
			gtk_widget_show(gauge);
		}
		g_array_free(classes,TRUE);
		classes = NULL;
		g_strfreev(files);
		g_list_foreach(p_list,free_element,NULL);
		g_list_foreach(s_list,free_element,NULL);
		g_list_free(p_list);
		g_list_free(s_list);
		p_list = NULL;
		s_list = NULL;
	}
	gtk_widget_show_all(table);
	gtk_widget_show_all(notebook);
}


G_MODULE_EXPORT gboolean gauge_choice_button_event(GtkWidget *widget, GdkEventButton *event, gpointer UNUSED(data))
{
	GtkWidget *table = NULL;
	gint i = 0;
	gint x_cur = 0;
	gint y_cur = 0;
	gint width = 0;
	gint height = 0;
	gint origin_x = 0;
	gint origin_y = 0;
	gint total_gauges = 0;
	GtkWidget * dash = NULL;
	GtkWidget * gauge = NULL;
	GtkTableChild *child = NULL;
	gchar * filename = NULL;
	gint row = 0;

	gdk_window_get_origin(gtk_widget_get_window(widget),&origin_x,&origin_y);
	gdk_drawable_get_size(gtk_widget_get_window(widget),&width,&height);
	
	/* Current cursor location relative to upper left corner */


	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	table = (GtkWidget *)OBJ_GET((widget),"table");

	if (GTK_IS_TABLE(table))
		total_gauges = g_list_length(GTK_TABLE(table)->children);
	else
		return FALSE;

	row = (gint)(((float)y_cur/(float)height)*(float)total_gauges);

	if (event->button == 1)
	{
		for (i=1;i<total_gauges;i++)
		{
			child = (GtkTableChild *) g_list_nth_data(GTK_TABLE(table)->children,i);
			if (row == child->top_attach)
			{
				if (GTK_IS_BOX(child->widget))
				{
					gauge = (GtkWidget *)OBJ_GET(child->widget, "gauge");
					filename = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge));
				}
			}
		}
		dash =  GTK_WIDGET(gtk_builder_get_object(toplevel,"dashboard"));
		gauge = mtx_gauge_face_new();
		gtk_fixed_put(GTK_FIXED(dash),gauge,130,130);
		if (!filename)
			printf("DASH PROBLEM, gauge filename is UNDEFINED for a gauge!\n");
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		gtk_widget_show_all(dash);
		g_free(filename);
		update_properties(gauge,GAUGE_ADD);
		changed = TRUE;
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);
	}

	/*printf("button event in gauge choice window at %i,%i\n",x_cur,y_cur);*/


	return TRUE;

}


G_MODULE_EXPORT gboolean dashdesigner_quit(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	extern GtkTreeStore *store;
	if (changed)
		prompt_to_save();
	g_object_unref(store);
	gtk_main_quit();
	return TRUE;
}



G_MODULE_EXPORT gboolean motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer UNUSED(data))
{
	gint x_cur;
	gint y_cur;
	gint origin_x;
	gint origin_y;

	gdk_window_get_origin(gtk_widget_get_window(widget),&origin_x,&origin_y);
	/* Current cursor locatio nrelatuive to upper left corner */


	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;


	/*printf("motion event\n");*/


	/*printf("rel movement point %i,%i\n",x_cur,y_cur);*/


	if (grabbed)
	{
		if (moving)
		{
			gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(grabbed_widget)),grabbed_widget,
					x_cur-tt.rel_grab_x+tt.child_x_origin,
					y_cur-tt.rel_grab_y+tt.child_y_origin);
		}
		if (resizing)
		{
			if (corner == LR)
			{
				gtk_widget_set_usize(grabbed_widget,MIN(event->x,event->y),MIN(event->x,event->y));
			}
			else if (corner == UR)
			{
				gtk_widget_set_usize(grabbed_widget,
						MIN(x_cur-tt.child_x_origin,(tt.child_y_origin+tt.child_height)-y_cur),
						MIN(x_cur-tt.child_x_origin,(tt.child_y_origin+tt.child_height)-y_cur));
						

				gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(grabbed_widget)),
						grabbed_widget,
						tt.child_x_origin,
						tt.child_y_origin+tt.child_height-MIN(x_cur-tt.child_x_origin,(tt.child_y_origin+tt.child_height)-y_cur));
			}
			else if (corner == UL)
			{
				gtk_widget_set_usize(grabbed_widget,
						MIN((tt.child_x_origin+tt.child_width)-x_cur,(tt.child_y_origin+tt.child_height)-y_cur),
						MIN((tt.child_x_origin+tt.child_width)-x_cur,(tt.child_y_origin+tt.child_height)-y_cur));

				gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(grabbed_widget)),
						grabbed_widget,
						tt.child_x_origin+tt.child_width-MIN((tt.child_x_origin+tt.child_width)-x_cur,(tt.child_y_origin+tt.child_height)-y_cur),
						tt.child_y_origin+tt.child_height-MIN((tt.child_x_origin+tt.child_width)-x_cur,(tt.child_y_origin+tt.child_height)-y_cur));
			}
			else if (corner == LL)
			{
				gtk_widget_set_usize(grabbed_widget,
						MIN(abs((tt.child_x_origin+tt.child_width)-x_cur),event->y),MIN(abs((tt.child_x_origin+tt.child_width)-x_cur),event->y));

				gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(grabbed_widget)),
						grabbed_widget,
						tt.child_x_origin+tt.child_width-MIN((tt.child_x_origin+tt.child_width)-x_cur,event->y),
						tt.child_y_origin);
			}
		}
		changed =  TRUE;
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
	        gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);
	}

	return TRUE;
}



G_MODULE_EXPORT gboolean button_event(GtkWidget *widget, GdkEventButton *event, gpointer UNUSED(data))
{
	GtkWidget * fixed = gtk_bin_get_child(GTK_BIN(widget));
	GList *list = NULL;
	gint i = 0;
	gint origin_x = 0;
	gint origin_y = 0;
	gboolean found_one = FALSE;
	gint x_cur = 0;
	gint y_cur = 0;
	GtkFixedChild * fchild = NULL;
	GdkWindow *win = NULL;
	gint len = g_list_length(GTK_FIXED(fixed)->children);
	GtkAllocation allocation;

	if (event->state & GDK_BUTTON1_MASK)
	{
		grabbed = FALSE;
		moving = FALSE;
		resizing = FALSE;
		corner = -1;
		/*printf("button1 released, unlocking\n");*/


		return TRUE;
	}

	gdk_window_get_origin(gtk_widget_get_window(widget),&origin_x,&origin_y);
	/* Current cursor locatio nrelatuive to upper left corner */


	x_cur = (gint)event->x_root-origin_x;
	y_cur = (gint)event->y_root-origin_y;

	if ((event->button))
	{
		win = gdk_window_at_pointer(NULL,NULL);
		list = g_list_first(GTK_FIXED(fixed)->children);
		found_one = FALSE;
		for (i=0;i<len;i++)
		{
			fchild = (GtkFixedChild *)g_list_nth_data(list,i);
			if (!fchild)
				continue;
			if (win == gtk_widget_get_window(fchild->widget))
			{
				gtk_widget_get_allocation(fchild->widget,&allocation);
				tt.child_x_origin = allocation.x;
				tt.child_y_origin = allocation.y;
				tt.child_width = allocation.width;
				tt.child_height = allocation.height;

				raise_fixed_child(fchild->widget);
				found_one = TRUE;
				mtx_gauge_face_set_show_drag_border(MTX_GAUGE_FACE(fchild->widget),TRUE);
				grabbed_widget = fchild->widget;

			}
			else
			{
				mtx_gauge_face_set_show_drag_border(MTX_GAUGE_FACE(fchild->widget),FALSE);
			}
		}
		if (found_one)
		{
			if (event->button == 3)
			{
				update_properties(grabbed_widget,GAUGE_REMOVE);
				gtk_widget_destroy(grabbed_widget);
				changed =  TRUE;
				gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
				gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
				gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);
			}
			if (event->button == 1)
			{
				/*printf("grabbed it \n");*/


				grabbed = TRUE;
				tt.rel_grab_x=x_cur;
				tt.rel_grab_y=y_cur;

				if (((x_cur < (tt.child_x_origin+DRAG_BORDER)) && ((y_cur < tt.child_y_origin+DRAG_BORDER) || (y_cur > tt.child_y_origin+tt.child_height-DRAG_BORDER))) || ((x_cur > (tt.child_x_origin+tt.child_width-DRAG_BORDER)) && ((y_cur < tt.child_y_origin+DRAG_BORDER) || (y_cur > tt.child_y_origin+tt.child_height-DRAG_BORDER))))
				{
					resizing = TRUE;
					/* Left border */


					if (x_cur < (tt.child_x_origin+7))
					{
						if (y_cur < (tt.child_y_origin+(0.33*tt.child_height)))
							corner = UL;
						else if (y_cur > (tt.child_y_origin+(0.66*tt.child_height)))
							corner = LL;
						else corner = -1;
						
					}
					/* Right border */


					else if (x_cur > (tt.child_x_origin+tt.child_width-7))
					{
						if (y_cur < (tt.child_y_origin+(0.33*tt.child_height)))
							corner = UR;
						else if (y_cur > (tt.child_y_origin+(0.66*tt.child_height)))
							corner = LR;
						else
							corner = -1;
					}
					/* Top border */


					if (y_cur < (tt.child_y_origin+7))
					{
						if (x_cur < (tt.child_x_origin+(0.33*tt.child_width)))
							corner = UL;
						else if (x_cur > (tt.child_x_origin+(0.66*tt.child_width)))
							corner = UR;
						else
							corner = -1;
					}
					/* Bottom border */


					else if (y_cur > (tt.child_y_origin+tt.child_height-7))
					{
						if (x_cur < (tt.child_x_origin+(0.33*tt.child_width)))
							corner = LL;
						else if (x_cur > (tt.child_x_origin+(0.66*tt.child_width)))
							corner = LR;
						else 
							corner = -1;
					}
				}
				else
					moving = TRUE;


			}
			else
			{
				/*printf("didn't grab squat\n");*/


				grabbed = FALSE;
				moving = FALSE;
				resizing = FALSE;
			}
		}
	}



	/*
	if (moving)
		printf ("MOVING!\n");
	if (resizing)
		printf ("RESIZING\n");
	if (grabbed)
	{
		printf("grabbed TRUE\n");
		printf("child base %i,%i, rel click point %i,%i\n",tt.child_x_origin,tt.child_y_origin,tt.rel_grab_x,tt.rel_grab_y);
	}
	*/


	
	return TRUE;

}

void raise_fixed_child (GtkWidget * widget)
{
	GtkFixed *fixed;

	g_return_if_fail (GTK_IS_FIXED (gtk_widget_get_parent(widget)));
	fixed = GTK_FIXED (gtk_widget_get_parent(widget));
	/* If widget hasn't got a window, move it to the back of the parent fixed's
	 *      children. If it has got a window, raise it. */


	/* Note: this is slightly naughty as it changes the GtkFixed's GList of
	 *      children, but it's better than removing the widget and adding it again. */


	if (gtk_widget_get_has_window (widget))
	{
		GList *child;
		GtkFixedChild *data;
		child = fixed->children;
		while (child)
		{
			data = (GtkFixedChild *)child->data;
			if (data->widget == widget)
			{
				fixed->children = g_list_remove (fixed->children, data);
				fixed->children = g_list_append (fixed->children, data);
				break;
			}
			child = child->next;
		}
	}
	else
	{
		gdk_window_raise (gtk_widget_get_window(widget));
	}
}


void update_properties(GtkWidget * widget, Choice choice)
{
	extern GtkTreeStore *store;
	GtkWidget * combo_box = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *table = NULL;
	gchar *tmpbuf = NULL;
	gchar **vector = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *sep = NULL;
	GtkEntryCompletion *completion = NULL;
	gint len = 0;

	g_return_if_fail(store);
	g_return_if_fail(GTK_IS_WIDGET(widget));

	if (choice == GAUGE_ADD)
	{
		changed =  TRUE;
		table = gtk_table_new(3,2,FALSE);
		gtk_container_set_border_width(GTK_CONTAINER(table),5);
		entry = gtk_entry_new();
		tmpbuf = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(widget));
		if (!tmpbuf)
			printf("ERROR, gauge has no associated filename!!\n");
		g_assert(tmpbuf);
		vector = g_strsplit(tmpbuf,PSEP,-1);
		len = g_strv_length(vector);
		g_free(tmpbuf);
		tmpbuf = g_strdelimit(g_strdup(vector[len-1]),"_",' ');
		g_strfreev(vector);
		vector = g_strsplit(tmpbuf,".",-1);
		g_free(tmpbuf);
	
		gtk_entry_set_text(GTK_ENTRY(entry),vector[0]);
		gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
		g_strfreev(vector);
		gtk_table_attach(GTK_TABLE(table),entry,0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),(GtkAttachOptions)GTK_FILL,0,0);

		combo_box = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(store),VARNAME_COL);
		completion = gtk_entry_completion_new();
		gtk_entry_set_completion(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_box))),completion);
		gtk_entry_completion_set_model(completion,GTK_TREE_MODEL(store));
		gtk_entry_completion_set_text_column(completion,VARNAME_COL);
                gtk_entry_completion_set_popup_single_match(completion,TRUE);
                gtk_entry_completion_set_inline_completion(completion,TRUE);
                gtk_entry_completion_set_inline_selection(completion,TRUE);
		OBJ_SET(combo_box,"arrow-size",GINT_TO_POINTER(1));
		gtk_table_attach(GTK_TABLE(table),combo_box,0,2,1,2,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),(GtkAttachOptions)GTK_FILL,0,0);
		OBJ_SET((widget),"combo",combo_box);
		OBJ_SET((combo_box),"model",store);

		sep = gtk_hseparator_new();
		gtk_table_attach(GTK_TABLE(table),sep,0,2,1,2,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),(GtkAttachOptions)GTK_FILL,0,5);
		
		if (OBJ_GET((widget),"datasource"))
			set_combo_to_source(combo_box,(gchar *)OBJ_GET((widget),"datasource"));

		vbox = GTK_WIDGET(gtk_builder_get_object(properties,"prop_top_vbox"));
		OBJ_SET((widget),"prop_table",table);
		gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,TRUE,0);
		gtk_widget_show_all(vbox);
	}
	else if (choice == GAUGE_REMOVE)
	{
		/*printf ("gauge removal\n");*/


		changed =  TRUE;
		table = (GtkWidget *)OBJ_GET((widget),"prop_table");
		gtk_widget_destroy(table);
	}
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);

	/*printf("update_properties\n");*/


}


void set_combo_to_source(GtkWidget *combo, gchar * source)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	GtkTreeIter parent;
	gboolean valid = FALSE;
	gboolean found = FALSE;
	gchar * potential;

	g_return_if_fail(combo);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
	g_return_if_fail(model);

	valid = gtk_tree_model_get_iter_first (model, &parent);
	while ((valid) && (!found))
	{
		if (gtk_tree_model_iter_has_child(model,&parent))
			valid = gtk_tree_model_iter_children(model,&iter,&parent);
		while ((valid) && (!found))
		{
			gtk_tree_model_get(model,&iter,DATASOURCE_COL,&potential,-1);
			if (!potential)
				goto again;
			/*if (g_ascii_strcasecmp(potential,source) == 0)*/
			if (g_strcmp0(potential,source) == 0)
			{
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter);
				found = TRUE;
			}
again:
			valid = gtk_tree_model_iter_next (model, &iter);
		}
		valid = gtk_tree_model_iter_next (model, &parent);
	}
	changed =  TRUE;
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);
}


G_MODULE_EXPORT gboolean close_current_dash(GtkWidget *UNUSED(widget), gchar * UNUSED(source))
{
	GtkWidget *dash = NULL;
	GtkWidget *topwidget = NULL;
	GList *children = NULL;
	GtkWidget * dialog = NULL;
	gint result = 0;
	dash = GTK_WIDGET(gtk_builder_get_object(toplevel,"dashboard"));

	children = GTK_FIXED(dash)->children;
	if ((changed) && (g_list_length(children) > 0))
		prompt_to_save();
	clear_dashboard(dash);
	topwidget = gtk_widget_get_toplevel(GTK_WIDGET(dash));
	gtk_window_resize(GTK_WINDOW(topwidget),320,200);
	changed = FALSE;
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),FALSE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),FALSE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),FALSE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"load_dash_menuitem"),TRUE);
	return TRUE;
}


G_MODULE_EXPORT gboolean dummy(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	return TRUE;
}

gint list_sort(gconstpointer a, gconstpointer b)
{
	gchar *a1 = (gchar *)a;
	gchar *b1 = (gchar *)b;
	return g_ascii_strcasecmp(a1,b1);
}

void free_element(gpointer data, gpointer UNUSED(user_data))
{
        gchar *a = (gchar *)data;
	if (a)
		g_free(a);
}


G_MODULE_EXPORT gboolean optimize_dash_size(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	GtkWidget *dash = NULL;
	GtkWidget *g = NULL;
	GtkWidget *topwidget = NULL;
	GList *children = NULL;
	gint w = 0;
	gint h = 0;
	guint i = 0;
	gint left = 0;
	gint right = 0;
	gint top = 0;
	gint bottom = 0;
	gint x_shrink = 0;
	gint y_shrink = 0;
	GtkFixedChild *child = NULL;
	GtkAllocation allocation;
	dash = GTK_WIDGET(gtk_builder_get_object(toplevel,"dashboard"));

	gtk_widget_get_allocation(dash,&allocation);

	w = allocation.width;
	h = allocation.height;
	left = w;
	right = 0;
	top = h;
	bottom = 0;
	children = GTK_FIXED(dash)->children;
	for (i=0;i<g_list_length(children);i++)
	{
		child = (GtkFixedChild *)g_list_nth_data(children,i);
		g = child->widget;
		gtk_widget_get_allocation(g,&allocation);
		left = MIN(left,allocation.x);
		right = MAX(right,allocation.x+allocation.width);
		top = MIN(top,allocation.y);
		bottom = MAX(bottom,allocation.y+allocation.height);
	}
	for (i=0;i<g_list_length(children);i++)
	{
		child = (GtkFixedChild *)g_list_nth_data(children,i);
		g = child->widget;
		gtk_widget_get_allocation(g,&allocation);
		gtk_fixed_move(GTK_FIXED(gtk_widget_get_parent(g)),g,
				allocation.x-left,
				allocation.y-top);
	}
	x_shrink = w-(right-left);
	y_shrink = h-(bottom-top);
	
	gtk_widget_set_size_request(GTK_WIDGET(dash),right-left,bottom-top);
	topwidget = gtk_widget_get_toplevel(GTK_WIDGET(dash));
	gtk_widget_get_allocation(topwidget,&allocation);
	gtk_window_resize(GTK_WINDOW(topwidget),allocation.width-x_shrink,allocation.height-y_shrink);
	changed = TRUE;
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"save_dash_as_menuitem"),TRUE);
	gtk_widget_set_sensitive((GtkWidget *)OBJ_GET(toplevel,"close_dash_menuitem"),TRUE);

	return TRUE;
}
