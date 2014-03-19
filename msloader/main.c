/*
 * Copyright (C) 2002-2011 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
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
  \file msloader/main.c
  \ingroup Loader
  \brief main code for the CLI loader tool: msloader
  \author David Andruczyk
  */

#include <fcntl.h>
#include <glib/gstdio.h>
#include <loader_common.h>
#include <hc08_loader.h>
#include <s12x_loader.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

/* Prototypes */
void usage_and_exit(gchar *);
void verify_args(gint , gchar **);
gint main(gint , gchar **);
gboolean message_handler(gpointer);
FirmwareType type = MS1;
/* Prototypes */

/*!
 \brief main() is the typical main function in a C program, it performs
 all core initialization, loading of all main parameters, initializing handlers
 and entering gtk_main to process events until program close
 \param argc (gint) count of command line arguments
 \param argv (char **) array of command line args
 \returns TRUE
 */
gint main(gint argc, gchar ** argv)
{
	gint port_fd = 0;
	gint file_fd = 0;
	
	verify_args(argc, argv);

	output(g_strdup_printf("MegaTunix msloader %s\n",VERSION),TRUE);

	/* If we got this far, all is good argument wise */
	if (!lock_port(argv[2]))
	{
		output((gchar *)"Could NOT LOCK Serial Port\nCheck for already running serial apps using the port\n",FALSE);
		exit(-1);
	}

	port_fd = open_port(argv[2]);
	if (port_fd > 0)
		output((gchar *)"Port successfully opened\n",FALSE);
	else
	{
		output((gchar *)"Could NOT open Port check permissions\n",FALSE);
		unlock_port();
		exit(-1);
	}
#ifdef __WIN32__
	file_fd = open(argv[3], O_RDWR | O_BINARY );
#else
	file_fd = g_open(argv[3],O_RDONLY,S_IRUSR);
#endif
	if (file_fd > 0 )
		output((gchar *)"Firmware file successfully opened\n",FALSE);
	else
	{
		output((gchar *)"Could NOT open firmware file, check permissions/paths\n",FALSE);
		close_port(port_fd);
		unlock_port();
		exit(-1);
	}
	if (type == MS1)
	{
		setup_port(port_fd, 9600);
		do_ms1_load(port_fd,file_fd);
	}
	else if (type == MS2)
	{
		setup_port(port_fd,115200);
		do_ms2_load(port_fd,file_fd);
	}

	else if (type == LIBREEMS)
	{
		setup_port(port_fd,115200);
		do_libreems_load(port_fd,file_fd);
	}

	flush_serial(port_fd,BOTH);
	close_port(port_fd);
	unlock_port();
	return (0) ;
}


void verify_args(gint argc, gchar **argv)
{
	/* Invalid arg count, abort */
	if (argc != 4)
		usage_and_exit(g_strdup("Invalid number of arguments!"));

	
	if (g_ascii_strcasecmp(argv[1],"MS1") == 0)
		type = MS1;
	else if (g_ascii_strcasecmp(argv[1],"MS2") == 0)
		type = MS2;
	else if (g_ascii_strcasecmp(argv[1],"LIBREEMS") == 0)
		type = LIBREEMS;
	else
		usage_and_exit(g_strdup_printf("Device type \"%s\" not recognized",argv[1]));

	if (!g_file_test(argv[2], G_FILE_TEST_EXISTS))
		usage_and_exit(g_strdup_printf("Port \"%s\" does NOT exist...",argv[2]));

	if (!g_file_test(argv[3], G_FILE_TEST_IS_REGULAR))
		usage_and_exit(g_strdup_printf("Filename \"%s\" does NOT exist...",argv[3]));
}

void usage_and_exit(gchar * msg)
{
	printf("\nERROR!!!\n - %s\n",msg);
	g_free(msg);
	printf("\nINVALID USAGE\n - msloader [MS1|MS2|LIBREEMS] /path/to/port /path/to/.s19\n\n");
	exit (-1);
}


void output(gchar *msg, gboolean free_it)
{
	printf("%s",msg);
	if (free_it)
		g_free(msg);
}

G_MODULE_EXPORT void progress_update(gfloat fraction)
{
	gint value = (gint)(fraction*100.0);
	static gint last = 0.0;

	if (value != last)
		printf("%i%% complete\n",value);
	last = value;
}
