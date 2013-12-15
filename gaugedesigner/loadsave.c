
#include <events.h>
#include <loadsave.h>
#include <getfiles.h>
#include <stdio.h>


extern gboolean hold_handlers;
extern GtkWidget *gauge;
extern gboolean direct_path;
gboolean changed = FALSE;
gboolean gauge_loaded = FALSE;


void prompt_to_save()
{
	GtkWidget *dialog = NULL;
	extern GtkWidget *main_window;
	gint result = 0;
	dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,"This gauge has been modified, Save it or discard the changes?");
	gtk_dialog_add_button(GTK_DIALOG(dialog),"Save Gauge", GTK_RESPONSE_OK);
	gtk_dialog_add_button(GTK_DIALOG(dialog),"Discard Changes", GTK_RESPONSE_CANCEL);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch (result)
	{
		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			save_handler(NULL,GINT_TO_POINTER(TRUE));
			break;
	}
	return;
}
G_MODULE_EXPORT gboolean load_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	MtxFileIO *fileio = NULL;
	gchar *filename = NULL;
	extern gchar *cwd;

	if (hold_handlers)
		return TRUE;

	fileio = g_new0(MtxFileIO ,1);
	if (cwd)
		fileio->absolute_path = g_strdup(cwd);
	else
		fileio->default_path = g_strdup("Gauges");
	fileio->title = g_strdup("Select Gauge to Open");
	fileio->action = GTK_FILE_CHOOSER_ACTION_OPEN;
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->shortcut_folders = g_strdup("Gauges");

	filename = choose_file(fileio);
	if (filename)
	{
		if (!gauge)
			create_new_gauge(widget,NULL);
		mtx_gauge_face_import_xml(MTX_GAUGE_FACE(gauge),filename);
		/*printf("loading gauge file %s\n",filename);*/
		update_attributes();
		g_free (filename);
		gauge_loaded = TRUE;
	}
	free_mtxfileio(fileio);
	return TRUE;
}


G_MODULE_EXPORT gboolean save_as_handler(GtkWidget *widget, gpointer UNUSED(data))
{
	save_handler(widget,GINT_TO_POINTER(TRUE));
	return TRUE;
}

G_MODULE_EXPORT gboolean save_handler(GtkWidget *UNUSED(widget), gpointer UNUSED(data))
{
	gchar * tmpbuf = NULL;
	gchar * filename = NULL;
	gchar *defdir = NULL;
	gchar **vector = NULL;
	gint len = 0;
	MtxFileIO *fileio = NULL;
	extern GtkWidget *main_window;

	if (!MTX_IS_GAUGE_FACE(gauge))
		return FALSE;

	if (hold_handlers)
		return TRUE;

	defdir = g_build_path(PSEP,HOME(), ".MegaTunix",GAUGES_DATA_DIR, NULL);


	fileio = g_new0(MtxFileIO ,1);
	fileio->parent = main_window;
	fileio->title = g_strdup("Save Dashboard to File");
	fileio->filter = g_strdup("*.*,All Files,*.xml,XML Files");
	fileio->default_filename = g_strdup("Untitled-Gauge.xml");
	fileio->default_extension = g_strdup("xml");
	fileio->action = GTK_FILE_CHOOSER_ACTION_SAVE;

	filename = mtx_gauge_face_get_xml_filename(MTX_GAUGE_FACE(gauge));
	if (!filename)
	{
		fileio->filename = NULL;
		fileio->default_path = g_strdup(GAUGES_DATA_DIR);
	}
	else
	{
		if (direct_path)
		{
			fileio->filename = g_build_path(PSEP,g_get_current_dir(),filename,NULL);
			vector = g_strsplit(filename,PSEP,-1);
			len = g_strv_length(vector);
			g_free(vector[len-1]);
			vector[len-1] = NULL;
			tmpbuf = g_strjoinv(PSEP,vector);
			fileio->default_path = g_build_path(PSEP,g_get_current_dir(),tmpbuf,NULL);
			g_free(tmpbuf);

			g_strfreev(vector);
		}
		else
		{

			fileio->filename = filename;
			tmpbuf = g_strrstr(filename,"Gauges");
			if (!tmpbuf)
				goto not_themed;
			vector = g_strsplit(tmpbuf,PSEP,-1);
			if (g_strv_length(vector) == 3) /* Themed gauge */
				fileio->default_path = g_build_path(PSEP,GAUGES_DATA_DIR,vector[1],NULL);
			else if (g_strv_length(vector) == 2)
			{
not_themed:
				fileio->default_path = g_strdup(GAUGES_DATA_DIR);
			}
			g_strfreev(vector);
		}
	}

	filename = choose_file(fileio);
	if (filename)
	{
		mtx_gauge_face_export_xml(MTX_GAUGE_FACE(gauge),filename);
		g_free (filename);
	}
	free_mtxfileio(fileio);
	changed = FALSE;
	return TRUE;

}

void setup_file_filters(GtkFileChooser *chooser)
{
	GtkFileFilter * filter = NULL;
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),"*.*");
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter),"All Files");
	gtk_file_chooser_add_filter(chooser,filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(filter),"*.xml");
	gtk_file_filter_set_name(GTK_FILE_FILTER(filter),"XML Files");
	gtk_file_chooser_add_filter(chooser,filter);
	gtk_file_chooser_set_filter(chooser,filter);
	return ;
}
