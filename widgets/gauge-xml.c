/*
 * Copyright (C) 2006 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Megasquirt gauge widget XML I/O
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */


#include <gauge.h>
#include <gauge-xml.h>
#include <gauge-private.h>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

/**
 * load_elements:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
load_elements(MtxGaugeFace *gauge, xmlNode * a_node)
{
	xmlNode *cur_node = NULL;
	MtxXMLFuncs *xml_funcs = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
	{
		if (cur_node->type == XML_ELEMENT_NODE) 
		{
//			printf("node type: Element, name: \"%s\"\n", cur_node->name);
			xml_funcs = NULL;
			xml_funcs = g_hash_table_lookup(gauge->xmlfunc_hash,cur_node->name);
			/* If current element name has a set of function 
			 * handlers AND this element has a child AND  the
			 * child of this elements is a text field. THEN
			 * call the import function handler passing in the text
			 * from the child and a pointer to the destination 
			 * variable to store that value into
			 */ 
			if (xml_funcs) 
				if ((cur_node->children) && (cur_node->children->type == XML_TEXT_NODE))
					xml_funcs->import_func(gauge,cur_node->children,xml_funcs->dest_var);

		}

		load_elements(gauge,cur_node->children);
	}
}


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
void testload(GtkWidget *widget)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	MtxGaugeFace *gauge = MTX_GAUGE_FACE(widget);

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

		/*parse the file and get the DOM */
		doc = xmlReadFile("output.xml", NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file output.xml\n");
	}

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);

	g_object_freeze_notify(G_OBJECT(gauge));
	mtx_gauge_face_remove_all_color_ranges(gauge);
	load_elements(gauge, root_element);
	g_object_thaw_notify(G_OBJECT(gauge));
	gtk_window_resize((GtkWindow *)widget->parent,gauge->w,gauge->h);
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

}

void output_xml(GtkWidget * widget)
{
	gint i = 0;
	xmlDocPtr doc = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL;/* node pointers */
	xmlDtdPtr dtd = NULL;       /* DTD pointer */
	MtxGaugeFace *gauge = MTX_GAUGE_FACE(widget);
	MtxDispatchHelper *helper = NULL;
	MtxXMLFuncs * xml_funcs = NULL;

	LIBXML_TEST_VERSION;

	/* 
	 * Creates a new document, a node and set it as a root node
	 */
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "gauge");
	xmlDocSetRootElement(doc, root_node);

	/*
	 * Creates a DTD declaration. Isn't mandatory. 
	 */
	dtd = xmlCreateIntSubset(doc, BAD_CAST "gauge", NULL, BAD_CAST "mtxgauge.dtd");

	/* 
	 * xmlNewChild() creates a new node, which is "attached" as child node
	 * of root_node node. 
	 */
	helper = g_new0(MtxDispatchHelper, 1);
	helper->gauge = gauge;
	helper->root_node = root_node;
	for (i=0;i<gauge->xmlfunc_array->len;i++)
	{
		xml_funcs = g_array_index(gauge->xmlfunc_array,MtxXMLFuncs *, i);
		helper->element_name = xml_funcs->varname;
		helper->src = xml_funcs->dest_var;
		xml_funcs->export_func(helper);
	}
	
	g_free(helper);
	/*
	*/


	xmlSaveFormatFileEnc("output.xml", doc, "utf-8", 1);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
}


void mtx_gauge_color_import(MtxGaugeFace *gauge,xmlNode *node, gpointer dest)
{
	if (!(node->type == XML_TEXT_NODE))
	{
		printf("node type is NOT text,  exiting\n");
		return;
	}
	gchar ** vector = NULL;
	GdkColor *color = NULL;

	vector = g_strsplit((gchar*)node->content," ", 0);
	color = dest;
	color->red = (guint16)g_ascii_strtod(vector[0],NULL);
	color->green = (guint16)g_ascii_strtod(vector[1],NULL);
	color->blue = (guint16)g_ascii_strtod(vector[2],NULL);
	g_strfreev(vector);
}

void mtx_gauge_gfloat_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	if (!(node->type == XML_TEXT_NODE))
		return;
	gfloat * var = NULL;
	var = dest;
	*var = g_ascii_strtod((gchar*)node->content,NULL);
}

void mtx_gauge_gint_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	if (!(node->type == XML_TEXT_NODE))
		return;
	gint * var = NULL;
	var = dest;
	*var = (gint)g_ascii_strtod((gchar*)node->content,NULL);
}

void mtx_gauge_gchar_import(MtxGaugeFace *gauge, xmlNode *node, gpointer dest)
{
	if (!(node->type == XML_TEXT_NODE))
		return;
	gchar * var  = dest;
	if (var != NULL)
		g_free(var);
	var = g_strdup((gchar*)node->content);
}

void mtx_gauge_color_range_import(MtxGaugeFace *gauge, xmlNode *a_node, gpointer dest)
{
	xmlNode *cur_node = NULL;
	MtxColorRange *range = NULL;

	range = g_new0(MtxColorRange, 1);
	cur_node = a_node;
	while (cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			if (g_strcasecmp((gchar *)cur_node->name,"lowpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node->children,&range->lowpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"highpoint") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node->children,&range->highpoint);
			if (g_strcasecmp((gchar *)cur_node->name,"lwidth") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node->children,&range->lwidth);
			if (g_strcasecmp((gchar *)cur_node->name,"inset") == 0)
				mtx_gauge_gfloat_import(gauge, cur_node->children,&range->inset);
			if (g_strcasecmp((gchar *)cur_node->name,"color") == 0)
				mtx_gauge_color_import(gauge, cur_node->children,&range->color);
		}
		cur_node = cur_node->next;
	}
	g_array_append_val(gauge->ranges,range);
}

void mtx_gauge_color_range_export(MtxDispatchHelper * helper)
{
	gint i = 0;
	gchar * tmpbuf = NULL;
	MtxColorRange *range = NULL;
	xmlNodePtr node = NULL;

	for (i=0;i<helper->gauge->ranges->len;i++)
	{
		range = g_array_index(helper->gauge->ranges,MtxColorRange *, i);
		node = xmlNewChild(helper->root_node, NULL, BAD_CAST "color_range",
				NULL );
		tmpbuf = g_strdup_printf("%f",range->lowpoint);
		xmlNewChild(node, NULL, BAD_CAST "lowpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->highpoint);
		xmlNewChild(node, NULL, BAD_CAST "highpoint",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->lwidth);
		xmlNewChild(node, NULL, BAD_CAST "lwidth",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%f",range->inset);
		xmlNewChild(node, NULL, BAD_CAST "inset",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);
		tmpbuf = g_strdup_printf("%i %i %i", 
				range->color.red, 
				range->color.green, 
				range->color.blue); 
		xmlNewChild(node, NULL, BAD_CAST "color",
				BAD_CAST tmpbuf);
		g_free(tmpbuf);

	}
}

void mtx_gauge_color_export(MtxDispatchHelper * helper)
{
	gchar * tmpbuf = NULL;
	GdkColor *color = helper->src;
	tmpbuf = g_strdup_printf("%i %i %i", 
			color->red, 
			color->green, 
			color->blue); 
	xmlNewChild(helper->root_node, NULL, BAD_CAST helper->element_name,
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}

void mtx_gauge_gfloat_export(MtxDispatchHelper * helper)
{
	gchar * tmpbuf = NULL;
	gfloat *val = (gfloat *)helper->src;
	tmpbuf = g_strdup_printf("%f",*val);
	xmlNewChild(helper->root_node, NULL, BAD_CAST helper->element_name,
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}

void mtx_gauge_gint_export(MtxDispatchHelper * helper)
{
	gchar * tmpbuf = NULL;
	gint *val = (gint *)helper->src;
	tmpbuf = g_strdup_printf("%i",*val);
	xmlNewChild(helper->root_node, NULL, BAD_CAST helper->element_name,
			BAD_CAST tmpbuf);
	g_free(tmpbuf);
}
void mtx_gauge_gchar_export(MtxDispatchHelper * helper)
{
	xmlNewChild(helper->root_node, NULL, BAD_CAST helper->element_name,
			BAD_CAST helper->src);
}


#endif
