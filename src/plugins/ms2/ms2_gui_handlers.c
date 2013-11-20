/*
 * Copyright (C) 2002-2012 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file src/plugins/ms2/ms2_gui_handlers.c
  \ingroup MS2Plugin,Plugins
  \brief MS2 Plugin Gui handler functions
  \author David Andruczyk
  */

#include <combo_loader.h>
#include <firmware.h>
#include <glade/glade.h>
#include <ms2_gui_handlers.h>
#include <ms2_plugin.h>
#include <ms2_tlogger.h>
#include <stdio.h>
#include <stdlib.h>
#include <user_outputs.h>


extern gconstpointer *global_data;


/*!
  \brief ECU specific text entry handler
  \param widget is a pointer to the widget the user modified
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_entry_handler(GtkWidget *widget, gpointer data)
{
	ENTER();
	MTXDBG(CRITICAL,_("ERROR handler NOT found for widget %s, command aborted! BUG!!!\n"),glade_get_widget_name(widget));
	EXIT();
	return TRUE;

}


/*!
  \brief ECU specific std button handler
  \param widget is a pointer to the widget the user modified
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_std_button_handler(GtkWidget *widget, gpointer data)
{
	gint handler = 0;
	gchar *tmpbuf = NULL;
	gfloat tmpf = 0.0;
	const gchar *dest = NULL;

	ENTER();
	handler = (MS2StdHandler)(GINT)OBJ_GET(widget,"handler");

	switch ((MS2StdHandler)handler)
	{
		case GET_CURR_TPS:
			tmpbuf = (gchar *)OBJ_GET(widget,"source");
			lookup_current_value_f(tmpbuf,&tmpf);
			dest = (const gchar *)OBJ_GET(widget,"dest_widget");
			tmpbuf = g_strdup_printf("%.0f",tmpf);
			gtk_entry_set_text(GTK_ENTRY(lookup_widget_f(dest)),tmpbuf);
			g_signal_emit_by_name(lookup_widget_f(dest),"activate",NULL);
			g_free(tmpbuf);
			break;
		default:
			MTXDBG(CRITICAL,_("ERROR handler NOT found for widget %s, command aborted! BUG!!!\n"),glade_get_widget_name(widget));
			break;
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific toggle button handler
  \param widget is a pointer to the widget the user modified
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_toggle_button_handler(GtkWidget *widget, gpointer data)
{
	extern MS2_TTMon_Data *ttm_data;
	gint handler = -1;
	
	ENTER();
	handler = (GINT)OBJ_GET(widget,"handler");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{       /* It's pressed (or checked) */
		switch ((MS2ToggleHandler)handler)
		{

			case START_TOOTHMON_LOGGER:
				ttm_data->stop = FALSE;
				OBJ_SET(ttm_data->darea,"io_cmd_function",(gpointer)"ms2_e_read_toothmon");
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("triggerlogger_buttons_table")),FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("compositelogger_buttons_table")),FALSE);
				bind_ttm_to_page((GINT)OBJ_GET(widget,"page"));
				io_cmd_f("ms2_e_read_toothmon",NULL);
				break;
			case START_TRIGMON_LOGGER:
				ttm_data->stop = FALSE;
				OBJ_SET(ttm_data->darea,"io_cmd_function",(gpointer)"ms2_e_read_trigmon");
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("toothlogger_buttons_table")),FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("compositelogger_buttons_table")),FALSE);
				bind_ttm_to_page((GINT)OBJ_GET(widget,"page"));
				io_cmd_f("ms2_e_read_trigmon",NULL);
				break;
			case START_COMPOSITEMON_LOGGER:
				ttm_data->stop = FALSE;
				OBJ_SET(ttm_data->darea,"io_cmd_function",(gpointer)"ms2_e_read_compositemon");
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("toothlogger_buttons_table")),NULL);
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("triggerlogger_buttons_table")),NULL);
				bind_ttm_to_page((GINT)OBJ_GET(widget,"page"));
				io_cmd_f("ms2_e_read_compositemon",NULL);
				break;
			case STOP_TOOTHMON_LOGGER:
				ttm_data->stop = TRUE;
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("triggerlogger_buttons_table")),TRUE);
				gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget_f("compositelogger_buttons_table")),TRUE);
				break;
			case STOP_TRIGMON_LOGGER:
				ttm_data->stop = TRUE;
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("toothlogger_buttons_table")),NULL);
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("compositelogger_buttons_table")),NULL);
				break;
			case STOP_COMPOSITEMON_LOGGER:
				ttm_data->stop = TRUE;
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("toothlogger_buttons_table")),NULL);
				alter_widget_state_f(GTK_WIDGET(lookup_widget_f("triggerlogger_buttons_table")),NULL);
				break;
			default:
				MTXDBG(CRITICAL,_("Default case reached,  i.e. handler not found in global, common or ECU plugins for widget %s, BUG!\n"),glade_get_widget_name(widget));
				break;
		}
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific combo box handler
  \param widget is a pointer to the widget the user modified
  \param data is unused
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_combo_handler(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	gint handler = 0;
	gint bitmask = 0;
	gint bitshift = 0;
	gint total = 0;
	guchar bitval = 0;
	gchar * set_labels = NULL;
	gchar * swap_list = NULL;
	gchar * tmpbuf = NULL;
	gchar * table_2_update = NULL;
	gchar * group_2_update = NULL;
	gchar * lower = NULL;
	gchar * upper = NULL;
	gfloat * multiplier = NULL;
	gfloat * adder = NULL;
	gint precision = 0;
	gchar ** vector = NULL;
	guint i = 0;
	gint tmpi = 0;
	gint page = 0;
	gint offset = 0;
	gint canID = 0;
	gint table_num = 0;
	gchar * range = NULL;
	DataSize size = MTX_U08;
	guint8 tmp = 0;
	gint dload_val = 0;
	gint dl_type = 0;
	gfloat real_lower = 0.0;
	gfloat real_upper = 0.0;
	gboolean temp_dep = FALSE;
	gfloat tmpf = 0.0;
	gfloat tmpf2 = 0.0;
	gboolean state = FALSE;
	Deferred_Data *d_data = NULL;
	GtkWidget *tmpwidget = NULL;
	void *eval = NULL;
	void (*check_limits)(gint) = NULL;

	ENTER();
	handler = (GINT)OBJ_GET(widget,"handler");
	dl_type = (GINT)OBJ_GET(widget,"dl_type");
	page = (GINT)OBJ_GET(widget,"page");
	offset = (GINT)OBJ_GET(widget,"offset");
	canID = (GINT)OBJ_GET(widget,"canID");
	if (!OBJ_GET(widget,"size"))
		size = MTX_U08 ;        /* default! */
	else
		size = (DataSize)(GINT)OBJ_GET(widget,"size");
	bitval = (GINT)OBJ_GET(widget,"bitval");
	bitmask = (GINT)OBJ_GET(widget,"bitmask");
	bitshift = get_bitshift_f(bitmask);

	state = gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (state == 0)
	{
		/* Not selected by combo popdown button, thus is being edited.
		   * Do a model scan to see if we actually hit the jackpot or
		   * not, and get the iter for it...
		   */
		if (!search_model_f(model,widget,&iter))
		{
			EXIT();
			return FALSE;
		}
	}
	gtk_tree_model_get(model,&iter,BITVAL_COL,&bitval,-1);

	switch (handler)
	{
		case MS2_USER_OUTPUTS:
			/* Send the offset */
			tmp = ms_get_ecu_data_f(canID,page,offset,size);
			tmp = tmp & ~bitmask;   /*clears bits */
			tmp = tmp | (bitval << bitshift);
			ms_send_to_ecu_f(canID, page, offset, size, tmp, TRUE);
			/* Get the rest of the data from the combo */
			gtk_tree_model_get(model,&iter,UO_TEMP_DEP_COL,&temp_dep,UO_SIZE_COL,&size,UO_RAW_LOWER_COL,&lower,UO_RAW_UPPER_COL,&upper,UO_REAL_LOWER_COL,&real_lower,UO_REAL_UPPER_COL,&real_upper,UO_RANGE_COL,&range,UO_PRECISION_COL,&precision,UO_FROMECU_MULT_COL,&multiplier,UO_FROMECU_ADD_COL,&adder,-1);

			/* Send the "size" of the offset to the ecu */
			if (OBJ_GET(widget,"size_offset"))
			{
				offset = (GINT)strtol((gchar *)OBJ_GET(widget,"size_offset"),NULL,10);
				ms_send_to_ecu_f(canID, page, offset, MTX_U08, size, TRUE);
			}
			else
				printf("size_offset NOT FOUND on widget %s\n",(gchar *)glade_get_widget_name(widget));

			tmpbuf = (gchar *)OBJ_GET(widget,"range_label");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_LABEL(tmpwidget))
			{
				if (temp_dep)
				{
					tmpbuf = g_strdup_printf("Valid Range: %.1f <-> %.1f",temp_to_host_f(real_lower),temp_to_host_f(real_upper));
					gtk_label_set_text(GTK_LABEL(tmpwidget),tmpbuf);
					g_free(tmpbuf);
				}
				else
					gtk_label_set_text(GTK_LABEL(tmpwidget),range);
			}

			tmpbuf = (gchar *)OBJ_GET(widget,"thresh_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);

			if (GTK_IS_WIDGET(tmpwidget))
			{
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(tmpwidget,"raw_lower",lower);
				OBJ_SET(tmpwidget,"raw_upper",upper);
				if (multiplier)
					OBJ_SET(tmpwidget,"fromecu_mult",multiplier);
				else
					OBJ_SET(tmpwidget,"fromecu_mult",NULL);
				if (adder)
					OBJ_SET(tmpwidget,"fromecu_add",adder);
				else
					OBJ_SET(tmpwidget,"fromecu_add",NULL);
				update_widget_f(tmpwidget,NULL);
			}
			tmpbuf = (gchar *)OBJ_GET(widget,"hyst_widget");
			if (tmpbuf)
				tmpwidget = lookup_widget_f(tmpbuf);
			if (GTK_IS_WIDGET(tmpwidget))
			{
				OBJ_SET(tmpwidget,"size",GINT_TO_POINTER(size));
				OBJ_SET(tmpwidget,"precision",GINT_TO_POINTER(precision));
				OBJ_SET(tmpwidget,"raw_lower",lower);
				OBJ_SET(tmpwidget,"raw_upper",upper);
				if (multiplier)
					OBJ_SET(tmpwidget,"fromecu_mult",multiplier);
				else
					OBJ_SET(tmpwidget,"fromecu_mult",NULL);
				if (adder)
					OBJ_SET(tmpwidget,"fromecu_add",adder);
				else
					OBJ_SET(tmpwidget,"fromecu_add",NULL);
				update_widget_f(tmpwidget,NULL);
			}
			g_free(range);
			EXIT();
			return TRUE;
			break;

		default:
			MTXDBG(CRITICAL,_("Default case reached,  i.e. handler not found in global, common or ECU plugins for widget %s, BUG!\n"),glade_get_widget_name(widget));
			EXIT();
			return TRUE;
			break;
	}
	if (dl_type == IMMEDIATE)
	{
		dload_val = convert_before_download_f(widget,dload_val);
		ms_send_to_ecu_f(canID, page, offset, size, dload_val, TRUE);
	}
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific combo update routine.  This is called from the common
  routine to handle any firmware specific stuff related to a ComboBoxEntry
  \param widget is the pointer to the GtkComboBox widget
  \returns TRUE
  */
G_MODULE_EXPORT gboolean ecu_update_combo(GtkWidget * widget)
{
	ENTER();
	if ((GINT)OBJ_GET(widget,"handler") == MS2_USER_OUTPUTS)
		update_ms2_user_outputs(widget);
	EXIT();
	return TRUE;
}


/*!
  \brief ECU specific gui init function to setup anything in the core gui for 
  this ECU family
  */
G_MODULE_EXPORT void ecu_gui_init(void)
{
	ENTER();
	EXIT();
	return;
	/* We don't need anything specific to this ecu initialized */
}
