/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <protos.h>
#include <defines.h>
#include <globals.h>
#include <constants.h>


extern int req_fuel_popup;
static int paused_handlers = FALSE;
extern int raw_reader_running;
extern int raw_reader_stopped;
extern int read_wait_time;
extern struct ms_ve_constants *ve_constants;
extern struct v1_2_Constants constants;
extern struct Reqd_Fuel reqd_fuel;

void leave(GtkWidget *widget, gpointer *data)
{
	save_config();
	stop_serial_thread();
	/* Free all buffers */
	close_serial();
	mem_dealloc();
	gtk_main_quit();
}

int text_entry_handler(GtkWidget * widget, gpointer *data)
{
	gchar *entry_text;
	gint offset;
	if (paused_handlers)
		return TRUE ;
	entry_text = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget));
	offset = (gint)gtk_object_get_data(G_OBJECT(widget),"offset");
	switch ((gint)data)
	{
		default:
			break;
	}
	/* update the widget in case data was out of bounds */
	return TRUE;
}



int std_button_handler(GtkWidget *widget, gpointer *data)
{
	switch ((gint)data)
	{
		case START_REALTIME:
			start_serial_thread();
			break;
		case STOP_REALTIME:
			stop_serial_thread();
			break;
		case REQD_FUEL_POPUP:
			if (!req_fuel_popup)
				reqd_fuel_popup();
			break;
		case READ_FROM_MS:
			paused_handlers = TRUE;
			read_ve_const();
			update_const_ve();
			paused_handlers = FALSE;
			break;
		case WRITE_TO_MS:
			//write_ve_const();
			break;
	}
	return TRUE;
}

void update_statusbar(GtkWidget *status_bar,int context_id, gchar * message)
{
	/* takes 3 args, 
	 * the GtkWidget pointer to the statusbar,
	 * the context_id of the statusbar in arg[0],
	 * and the string to be sent to that bar
	 *
	 * Fairly generic, works for multiple statusbars
	 */

	gtk_statusbar_pop(GTK_STATUSBAR(status_bar),
			context_id);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar),
			context_id,
			message);
}
	
int classed_spinner_changed(GtkWidget *widget, gpointer *data)
{
        gfloat value = 0.0;
        gint offset = 0;
	gint class = 0;
        if (paused_handlers)
                return TRUE;
        value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	/* Class is set to determine the course of action */
	class = (gint) gtk_object_get_data(G_OBJECT(widget),"class");
	offset = (gint) gtk_object_get_data(G_OBJECT(widget),"offset");
	switch (class)
	{
		case WARMUP:
			ve_constants->warmup_bins[offset-WARMUP_BINS_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case RPM:
			ve_constants->rpm_bins[offset-RPM_BINS_OFFSET] 
				= (gint)value/100.0;
			write_ve_const((gint)value/100.0, offset);
			break;
		case KPA:
			ve_constants->kpa_bins[offset-KPA_BINS_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case VE:
			ve_constants->ve_bins[offset-VE_TABLE_OFFSET] 
				= (gint)value;
			write_ve_const((gint)value, offset);
			break;
		case ACCEL:
			ve_constants->accel_bins[offset-ACCEL_BINS_OFFSET] 
				= (gint)((value*10.0)+.01);
			write_ve_const((gint)((value*10.0)+.01), offset);
			break;
	}

	return TRUE;
}

int spinner_changed(GtkWidget *widget, gpointer *data)
{
	/* Gets the value from the spinbutton then modifues the 
	 * necessary deta in the the app and calls any handlers 
	 * if necessary.  works well,  one generic function with a 
	 * select/case branch to handle the choices..
	 */
	gfloat value = 0.0;
	gint offset;
	gint tmpi_x10 = 0; /* value*10 converted to an INT */
	gint tmpi = 0;
	if (paused_handlers)
		return TRUE;
	value = (float)gtk_spin_button_get_value((GtkSpinButton *)widget);
	offset = (gint) gtk_object_get_data(G_OBJECT(widget),"offset");
	tmpi_x10 = (int)((value*10.0)+.001);
	tmpi = (int)(value+.001);

	switch ((gint)data)
	{
		case SET_SER_PORT:
			if(serial_params.open)
			{
				if (raw_reader_running)
					stop_serial_thread();
				close_serial();
			}
			open_serial((int)value);
			setup_serial_params();
			break;
		case SER_POLL_TIMEO:
			serial_params.poll_timeout = (gint)value;
			break;
		case SER_INTERVAL_DELAY:
			serial_params.read_wait = (gint)value;
			break;
		case REQ_FUEL_DISP:
			reqd_fuel.disp = (gint)value;
			break;
		case REQ_FUEL_CYLS:
			reqd_fuel.cyls = (gint)value;
			break;
		case REQ_FUEL_INJ_RATE:
			reqd_fuel.inj_rate = (gint)value;
			break;
		case REQ_FUEL_AFR:
			reqd_fuel.afr = value;
			break;
		case REQ_FUEL_1:
			ve_constants->req_fuel_1 = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case INJ_OPEN_TIME:
			/* This funny conversion is needed cause of 
			 * some weird quirk when multiplying the float
			 * by ten and then converting to int (is off by
			 * one in SOME cases only..
			 */
			ve_constants->inj_open_time = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case BATT_CORR:
			ve_constants->batt_corr = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case PWM_CUR_LIM:
			ve_constants->pwm_curr_lim = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case PWM_TIME_THRES:
			ve_constants->pwm_time_max = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case FAST_IDLE_THRES:
			ve_constants->fast_idle_thresh = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case CRANK_PULSE_NEG_40:
			ve_constants->cr_pulse_neg40 = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case CRANK_PULSE_170:
			ve_constants->cr_pulse_pos170 = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case CRANK_PRIMING_PULSE:
			ve_constants->cr_priming_pulse = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case AFTERSTART_ENRICH:
			ve_constants->as_enrich = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case AFTERSTART_NUM_CYCLES:
			ve_constants->as_num_cycles = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case TPS_TRIG_THRESH:
			tmpi = (int)((value*5.0)+.001);
			ve_constants->tps_trig_thresh = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case ACCEL_ENRICH_DUR:
			ve_constants->accel_duration = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case COLD_ACCEL_ENRICH:
			ve_constants->cold_accel_addon = tmpi_x10;
			write_ve_const(tmpi_x10, offset);
			break;
		case COLD_ACCEL_MULT:
			ve_constants->cold_accel_mult = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case DECEL_CUT:
			ve_constants->decel_cut = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case EGO_TEMP_ACTIVE:
			ve_constants->ego_temp_active = tmpi+40;
			write_ve_const(tmpi+40, offset);
			break;
		case EGO_RPM_ACTIVE:
			ve_constants->ego_rpm_active = tmpi/100;
			write_ve_const(tmpi/100, offset);
			break;
		case EGO_SW_VOLTAGE:	
			tmpi = (int)((value*51.0)+.001);
			ve_constants->ego_sw_voltage = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case EGO_STEP:	
			ve_constants->ego_step = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case EGO_EVENTS:	
			ve_constants->ego_events = tmpi;
			write_ve_const(tmpi, offset);
			break;
		case EGO_LIMIT:	
			ve_constants->ego_limit = tmpi;
			write_ve_const(tmpi, offset);
			break;
		default:
			break;
	}
	return TRUE;

}

void update_const_ve()
{
	gint i;

	/* req-fuel  */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.req_fuel_1_spin),
			ve_constants->req_fuel_1/5.0);

	/* inj_open_time */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.inj_open_time_spin),
			ve_constants->inj_open_time/10.0);

	/* batt_corr */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.batt_corr_spin),
			ve_constants->batt_corr/60.0);

	/* pwm_curr_lim */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.pwm_curr_lim_spin),
			ve_constants->pwm_curr_lim);

	/* pwm_time_thresh */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.pwm_time_max_spin),
			ve_constants->pwm_time_max/10.0);

	/* fast_idle_thresh */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.fast_idle_thresh_spin),
			ve_constants->fast_idle_thresh-40.0);

	/* crank pulse -40 */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_pulse_neg40_spin),
			ve_constants->cr_pulse_neg40/10.0);

	/* crank pulse 170 */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_pulse_pos170_spin),
			ve_constants->cr_pulse_pos170/10.0);

	/* Priming pulse */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.cr_priming_pulse_spin),
			ve_constants->cr_priming_pulse/10.0);

	/* Afterstart Enrich % */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.as_enrich_spin),
			ve_constants->as_enrich);

	/* Afterstart Enrich # of cycles */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(constants.as_num_cycles_spin),
			ve_constants->as_num_cycles);

	/* Warmup Enrichment bins */
	for (i=0;i<10;i++)
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.warmup_bins_spin[i]),
			ve_constants->warmup_bins[i]);
	}
	/* accel enrichments */
	for (i=0;i<4;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.accel_bins_spin[i]),
				ve_constants->accel_bins[i]/10.0);
	}

	/* TPS Trigger Threshold */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.tps_trig_thresh_spin),
			ve_constants->tps_trig_thresh/5.0);
//	g_snprintf(buff,10,"%.2f",ve_constants->tps_trig_thresh/5.0);
//	gtk_entry_set_text(GTK_ENTRY(constants.tps_trig_thresh_ent),
//			buff);

	/* Accel Enrich Duration */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.accel_duration_spin),
			ve_constants->accel_duration/10.0);
//	g_snprintf(buff,10,"%.1f",ve_constants->accel_duration/10.0);
//	gtk_entry_set_text(GTK_ENTRY(constants.accel_duration_ent),
//			buff);

	/* Cold Accel Enrich Add On */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.cold_accel_addon_spin),
			ve_constants->cold_accel_addon/10.0);
//	g_snprintf(buff,10,"%.1f",ve_constants->cold_accel_addon/10.0);
//	gtk_entry_set_text(GTK_ENTRY(constants.cold_accel_addon_ent),
//			buff);

	/* Cold Accel Enrich Multiplier */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.cold_accel_mult_spin),
			ve_constants->cold_accel_mult);
//	g_snprintf(buff,10,"%i",ve_constants->cold_accel_mult);
//	gtk_entry_set_text(GTK_ENTRY(constants.cold_accel_mult_ent),
//			buff);

	/* Decel Fuel Cut*/
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.decel_cut_spin),
			ve_constants->decel_cut);
//	g_snprintf(buff,10,"%i",ve_constants->decel_cut);
//	gtk_entry_set_text(GTK_ENTRY(constants.decel_cut_ent),
//			buff);

	/* EGO coolant activation temp */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_temp_active_spin),
			ve_constants->ego_temp_active-40);
//	g_snprintf(buff,10,"%i",ve_constants->ego_temp_active-40);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_temp_active_ent),
//			buff);

	/* EGO activation RPM */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_rpm_active_spin),
			ve_constants->ego_rpm_active*100);
//	g_snprintf(buff,10,"%i",ve_constants->ego_rpm_active*100);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_rpm_active_ent),
//			buff);

	/* EGO switching voltage */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_sw_voltage_spin),
			ve_constants->ego_sw_voltage/51.0);
//	g_snprintf(buff,10,"%.2f",(ve_constants->ego_sw_voltage/255.0)*5);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_sw_voltage_ent),
//			buff);

	/* EGO step percent */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_step_spin),
			ve_constants->ego_step);
//	g_snprintf(buff,10,"%i",ve_constants->ego_step);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_step_ent),
//			buff);

	/* EGO events between steps */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_events_spin),
			ve_constants->ego_events);
//	g_snprintf(buff,10,"%i",ve_constants->ego_events);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_events_ent),
//			buff);

	/* EGO limit % between steps */
	gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(constants.ego_limit_spin),
			ve_constants->ego_limit);
//	g_snprintf(buff,10,"%i",ve_constants->ego_limit);
//	gtk_entry_set_text(GTK_ENTRY(constants.ego_limit_ent),
//			buff);

	/* VE table entries */
	for (i=0;i<64;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.ve_bins_spin[i]),
				ve_constants->ve_bins[i]);
	}

	/* KPA axis VE table entries */
	for (i=0;i<8;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.kpa_bins_spin[i]),
				ve_constants->kpa_bins[i]);
	}

	/* RPM axis VE table entries */
	for (i=0;i<8;i++)
	{
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(constants.rpm_bins_spin[i]),
				ve_constants->rpm_bins[i]*100);
	}

	// Stub function, does nothing yet... 
}
