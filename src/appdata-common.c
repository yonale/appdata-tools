/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>

#include "appdata-common.h"

typedef enum {
	APPDATA_SECTION_UNKNOWN,
	APPDATA_SECTION_APPLICATION,
	APPDATA_SECTION_DESCRIPTION,
	APPDATA_SECTION_DESCRIPTION_PARA,
	APPDATA_SECTION_DESCRIPTION_UL,
	APPDATA_SECTION_DESCRIPTION_UL_LI,
	APPDATA_SECTION_ID,
	APPDATA_SECTION_LICENCE,
	APPDATA_SECTION_NAME,
	APPDATA_SECTION_SCREENSHOT,
	APPDATA_SECTION_SCREENSHOTS,
	APPDATA_SECTION_SUMMARY,
	APPDATA_SECTION_UPDATECONTACT,
	APPDATA_SECTION_URL,
	APPDATA_SECTION_LAST
} AppdataSection;

/**
 * appdata_selection_from_string:
 */
static AppdataSection
appdata_selection_from_string (const gchar *element_name)
{
	if (g_strcmp0 (element_name, "application") == 0)
		return APPDATA_SECTION_APPLICATION;
	if (g_strcmp0 (element_name, "id") == 0)
		return APPDATA_SECTION_ID;
	if (g_strcmp0 (element_name, "licence") == 0)
		return APPDATA_SECTION_LICENCE;
	if (g_strcmp0 (element_name, "screenshots") == 0)
		return APPDATA_SECTION_SCREENSHOTS;
	if (g_strcmp0 (element_name, "screenshot") == 0)
		return APPDATA_SECTION_SCREENSHOT;
	if (g_strcmp0 (element_name, "name") == 0)
		return APPDATA_SECTION_NAME;
	if (g_strcmp0 (element_name, "summary") == 0)
		return APPDATA_SECTION_SUMMARY;
	if (g_strcmp0 (element_name, "url") == 0)
		return APPDATA_SECTION_URL;
	if (g_strcmp0 (element_name, "description") == 0)
		return APPDATA_SECTION_DESCRIPTION;
	if (g_strcmp0 (element_name, "p") == 0)
		return APPDATA_SECTION_DESCRIPTION_PARA;
	if (g_strcmp0 (element_name, "ul") == 0)
		return APPDATA_SECTION_DESCRIPTION_UL;
	if (g_strcmp0 (element_name, "li") == 0)
		return APPDATA_SECTION_DESCRIPTION_UL_LI;
	if (g_strcmp0 (element_name, "updatecontact") == 0)
		return APPDATA_SECTION_UPDATECONTACT;
	return APPDATA_SECTION_UNKNOWN;
}

/**
 * appdata_selection_to_string:
 */
static const gchar *
appdata_selection_to_string (AppdataSection section)
{
	if (section == APPDATA_SECTION_APPLICATION)
		return "application";
	if (section == APPDATA_SECTION_ID)
		return "id";
	if (section == APPDATA_SECTION_LICENCE)
		return "licence";
	if (section == APPDATA_SECTION_SCREENSHOTS)
		return "screenshots";
	if (section == APPDATA_SECTION_SCREENSHOT)
		return "screenshot";
	if (section == APPDATA_SECTION_NAME)
		return "name";
	if (section == APPDATA_SECTION_SUMMARY)
		return "summary";
	if (section == APPDATA_SECTION_URL)
		return "url";
	if (section == APPDATA_SECTION_DESCRIPTION)
		return "description";
	if (section == APPDATA_SECTION_DESCRIPTION_PARA)
		return "p";
	if (section == APPDATA_SECTION_DESCRIPTION_UL)
		return "ul";
	if (section == APPDATA_SECTION_DESCRIPTION_UL_LI)
		return "li";
	if (section == APPDATA_SECTION_UPDATECONTACT)
		return "updatecontact";
	return NULL;
}

/**
 * appdata_add_problem:
 */
static void
appdata_add_problem (GList **problems, const gchar *str)
{
	GList *l;

	/* find if it's already been added */
	for (l = *problems; l != NULL; l = l->next) {
		if (g_strcmp0 (str, l->data) == 0)
			return;
	}

	/* add new problem to list */
	*problems = g_list_prepend (*problems, g_strdup (str));
}

typedef struct {
	AppdataSection	 section;
	gchar		*id;
	gchar		*name;
	gchar		*summary;
	gchar		*licence;
	gchar		*updatecontact;
	gchar		*url;
	GList		**problems;
	guint		 number_paragraphs;
	guint		 number_screenshots;
} AppdataHelper;

/**
 * appdata_start_element_fn:
 */
static void
appdata_start_element_fn (GMarkupParseContext *context,
			  const gchar *element_name,
			  const gchar **attribute_names,
			  const gchar **attribute_values,
			  gpointer user_data,
			  GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	AppdataSection new;
	const gchar *tmp;
	guint i;

	new = appdata_selection_from_string (element_name);

	/* unknown -> application */
	if (helper->section == APPDATA_SECTION_UNKNOWN) {
		if (new == APPDATA_SECTION_APPLICATION) {
			/* valid */
			helper->section = new;
			return;
		}
		g_set_error (error, 1, 0,
			     "start tag '%s' not allowed from section '%s'",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	/* application -> various */
	if (helper->section == APPDATA_SECTION_APPLICATION) {
		switch (new) {
		case APPDATA_SECTION_ID:
			tmp = NULL;
			for (i = 0; attribute_names[i] != NULL; i++) {
				if (g_strcmp0 (attribute_names[i], "type") == 0) {
					tmp = attribute_values[i];
					break;
				}
			}
			if (tmp == NULL)
				appdata_add_problem (helper->problems, "no type attribute in <id>");
			if (g_strcmp0 (tmp, "desktop") != 0)
				appdata_add_problem (helper->problems, "<id> has invalid type attribute");
			helper->section = new;
			break;
		case APPDATA_SECTION_URL:
			tmp = NULL;
			for (i = 0; attribute_names[i] != NULL; i++) {
				if (g_strcmp0 (attribute_names[i], "type") == 0) {
					tmp = attribute_values[i];
					break;
				}
			}
			if (tmp == NULL)
				appdata_add_problem (helper->problems, "no type attribute in <url>");
			if (g_strcmp0 (tmp, "homepage") != 0)
				appdata_add_problem (helper->problems, "<url> has invalid type attribute");
			helper->section = new;
			break;
		case APPDATA_SECTION_NAME:
		case APPDATA_SECTION_SUMMARY:
		case APPDATA_SECTION_LICENCE:
		case APPDATA_SECTION_DESCRIPTION:
		case APPDATA_SECTION_SCREENSHOTS:
		case APPDATA_SECTION_UPDATECONTACT:
			/* valid */
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag '%s' not allowed from section '%s'",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* description -> p or -> ul */
	if (helper->section == APPDATA_SECTION_DESCRIPTION) {
		switch (new) {
		case APPDATA_SECTION_DESCRIPTION_PARA:
			helper->section = new;
			break;
		case APPDATA_SECTION_DESCRIPTION_UL:
			/* ul without a leading para */
			if (helper->number_paragraphs < 1)
				appdata_add_problem (helper->problems, "<ul> cannot start a description");
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag '%s' not allowed from section '%s'",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* ul -> li */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL) {
		switch (new) {
		case APPDATA_SECTION_DESCRIPTION_UL_LI:
			/* valid */
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag '%s' not allowed from section '%s'",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* unknown -> application */
	if (helper->section == APPDATA_SECTION_SCREENSHOTS) {
		if (new == APPDATA_SECTION_SCREENSHOT) {
			/* valid */
			helper->number_screenshots++;
			helper->section = new;
			return;
		}
		g_set_error (error, 1, 0,
			     "start tag '%s' not allowed from section '%s'",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	g_set_error (error, 1, 0,
		     "start tag '%s' not allowed from section '%s'",
		     element_name,
		     appdata_selection_to_string (helper->section));
}

/**
 * appdata_end_element_fn:
 */
static void
appdata_end_element_fn (GMarkupParseContext *context,
			const gchar *element_name,
			gpointer user_data,
			GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	AppdataSection new;

	new = appdata_selection_from_string (element_name);
	if (helper->section == APPDATA_SECTION_APPLICATION) {
		if (new == APPDATA_SECTION_APPLICATION) {
			/* valid */
			helper->section = APPDATA_SECTION_UNKNOWN;
			return;
		}
		g_set_error (error, 1, 0,
			     "end tag '%s' not allowed from section '%s'",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	/* </id> */
	if (helper->section == APPDATA_SECTION_ID &&
	    new == APPDATA_SECTION_ID) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </licence> */
	if (helper->section == APPDATA_SECTION_LICENCE &&
	    new == APPDATA_SECTION_LICENCE) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </p> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_PARA &&
	    new == APPDATA_SECTION_DESCRIPTION_PARA) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION;
		return;
	}

	/* </li> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL_LI &&
	    new == APPDATA_SECTION_DESCRIPTION_UL_LI) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION_UL;
		return;
	}

	/* </ul> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL &&
	    new == APPDATA_SECTION_DESCRIPTION_UL) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION;
		return;
	}

	/* </description> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION &&
	    new == APPDATA_SECTION_DESCRIPTION) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </screenshot> */
	if (helper->section == APPDATA_SECTION_SCREENSHOT &&
	    new == APPDATA_SECTION_SCREENSHOT) {
		/* valid */
		helper->section = APPDATA_SECTION_SCREENSHOTS;
		return;
	}

	/* </screenshots> */
	if (helper->section == APPDATA_SECTION_SCREENSHOTS &&
	    new == APPDATA_SECTION_SCREENSHOTS) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </url> */
	if (helper->section == APPDATA_SECTION_URL &&
	    new == APPDATA_SECTION_URL) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </name> */
	if (helper->section == APPDATA_SECTION_NAME &&
	    new == APPDATA_SECTION_NAME) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </summary> */
	if (helper->section == APPDATA_SECTION_SUMMARY &&
	    new == APPDATA_SECTION_SUMMARY) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </updatecontact> */
	if (helper->section == APPDATA_SECTION_UPDATECONTACT &&
	    new == APPDATA_SECTION_UPDATECONTACT) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	g_set_error (error, 1, 0,
		     "end tag '%s' not allowed from section '%s'",
		     element_name,
		     appdata_selection_to_string (helper->section));
}

/**
 * appdata_text_fn:
 */
static void
appdata_text_fn (GMarkupParseContext *context,
		 const gchar *text,
		 gsize text_len,
		 gpointer user_data,
		 GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	gchar *temp;

	switch (helper->section) {
	case APPDATA_SECTION_ID:
		helper->id = g_strndup (text, text_len);
		g_strchomp (helper->id);
		if (!g_str_has_suffix (helper->id, ".desktop"))
			appdata_add_problem (helper->problems, "<id> does not end in 'desktop'");
		break;
	case APPDATA_SECTION_LICENCE:
		if (helper->licence != NULL) {
			g_free (helper->licence);
			appdata_add_problem (helper->problems, "<licence> is duplicated");
		}
		helper->licence = g_strndup (text, text_len);
		g_strchomp (helper->licence);
		if (g_strcmp0 (helper->licence, "CC0") != 0 &&
		    g_strcmp0 (helper->licence, "CC BY") != 0 &&
		    g_strcmp0 (helper->licence, "CC BY-SA") != 0)
			appdata_add_problem (helper->problems, "<licence> is not valid");
		break;
	case APPDATA_SECTION_URL:
		if (helper->url != NULL) {
			g_free (helper->url);
			appdata_add_problem (helper->problems, "<url> is duplicated");
		}
		helper->url = g_strndup (text, text_len);
		g_strchomp (helper->url);
		if (!g_str_has_prefix (helper->url, "http://") &&
		    !g_str_has_prefix (helper->url, "https://"))
			appdata_add_problem (helper->problems, "<url> does not start with 'http://'");
		break;
	case APPDATA_SECTION_UPDATECONTACT:
		if (helper->updatecontact != NULL) {
			g_free (helper->updatecontact);
			appdata_add_problem (helper->problems, "<updatecontact> is duplicated");
		}
		helper->updatecontact = g_strndup (text, text_len);
		g_strchomp (helper->updatecontact);
		if (g_strcmp0 (helper->updatecontact, "someone_who_cares@upstream_project.org") == 0)
			appdata_add_problem (helper->problems, "<updatecontact> is still set to a dummy value");
		if (strlen (helper->updatecontact) < 6)
			appdata_add_problem (helper->problems, "<updatecontact> is too short");
		break;
	case APPDATA_SECTION_NAME:
		if (helper->name != NULL) {
			g_free (helper->name);
			appdata_add_problem (helper->problems, "<name> is duplicated");
		}
		helper->name = g_strndup (text, text_len);
		g_strchomp (helper->name);
		if (strlen (helper->name) < 4)
			appdata_add_problem (helper->problems, "<name> is too short");
		break;
	case APPDATA_SECTION_SUMMARY:
		if (helper->summary != NULL) {
			g_free (helper->summary);
			appdata_add_problem (helper->problems, "<summary> is duplicated");
		}
		helper->summary = g_strndup (text, text_len);
		g_strchomp (helper->summary);
		if (strlen (helper->summary) < 8)
			appdata_add_problem (helper->problems, "<summary> is too short");
		break;
	case APPDATA_SECTION_DESCRIPTION_PARA:
		temp = g_strndup (text, text_len);
		g_strchomp (temp);
		if (strlen (temp) < 50)
			appdata_add_problem (helper->problems, "<p> is too short");
		g_free (temp);
		helper->number_paragraphs++;
		break;
	case APPDATA_SECTION_DESCRIPTION_UL_LI:
		temp = g_strndup (text, text_len);
		g_strchomp (temp);
		if (strlen (temp) < 25)
			appdata_add_problem (helper->problems, "<li> is too short");
		g_free (temp);
		break;
	default:
		/* ignore */
		break;
	}
}

/**
 * appdata_check_file_for_problems:
 */
GList *
appdata_check_file_for_problems (const gchar *filename, AppdataCheck check)
{
	gchar *data;
	gboolean ret;
	gsize data_len;
	GList *problems = NULL;
	GError *error = NULL;
	AppdataHelper *helper = NULL;
	GMarkupParseContext *context = NULL;
	const GMarkupParser parser = {
		appdata_start_element_fn,
		appdata_end_element_fn,
		appdata_text_fn,
		NULL,
		NULL };

	g_return_val_if_fail (filename != NULL, NULL);

	/* check file has the correct ending */
	if (!g_str_has_suffix (filename, ".appdata.xml"))
		appdata_add_problem (&problems, "incorrect extension, expected '.appdata.xml'");

	ret = g_file_get_contents (filename, &data, &data_len, &error);
	if (!ret) {
		appdata_add_problem (&problems, error->message);
		g_error_free (error);
		goto out;
	}

	/* parse */
	helper = g_new0 (AppdataHelper, 1);
	helper->problems = &problems;
	helper->section = APPDATA_SECTION_UNKNOWN;
	context = g_markup_parse_context_new (&parser, 0, helper, NULL);
	ret = g_markup_parse_context_parse (context, data, data_len, &error);
	if (!ret) {
		appdata_add_problem (&problems, error->message);
		g_error_free (error);
		goto out;
	}

	/* check for things that have to exist */
	if (helper->id == NULL)
		appdata_add_problem (&problems, "<id> is not present");
	if ((check & APPDATA_CHECK_ALLOW_MISSING_CONTACTDETAILS) == 0 &&
	    helper->updatecontact == NULL)
		appdata_add_problem (&problems, "<updatecontact> is not present");
	if (helper->url == NULL)
		appdata_add_problem (&problems, "<url> is not present");
	if (helper->licence == NULL)
		appdata_add_problem (&problems, "<licence> is not present");
	if (helper->number_paragraphs < 2)
		appdata_add_problem (&problems, "Not enough <p> tags for a good description");
	if (helper->number_screenshots < 1)
		appdata_add_problem (&problems, "Not enough <screenshot> tags");
	if (helper->summary != NULL && helper->name != NULL &&
	    strlen (helper->summary) < strlen (helper->name))
		appdata_add_problem (&problems, "<summary> is shorter than <name>");
out:
	if (helper != NULL) {
		g_free (helper->id);
		g_free (helper->licence);
		g_free (helper->url);
		g_free (helper->name);
		g_free (helper->summary);
		g_free (helper->updatecontact);
	}
	g_free (helper);
	g_free (data);
	if (context != NULL)
		g_markup_parse_context_unref (context);
	return problems;
}
