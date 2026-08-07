/* SPDX-License-Identifier: MIT */

#include "sessionmanager-xml.h"

typedef struct
{
	SessionmanagerSessionStore *store;
	SessionmanagerSession *current_session;
} SessionmanagerXmlParserData;

static const gchar *
sessionmanager_xml_find_attribute (
	const gchar       **attribute_names,
	const gchar       **attribute_values,
	const gchar        *name
)
{
	guint i;

	if (attribute_names == NULL || attribute_values == NULL)
	{
		return NULL;
	}

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		if (g_strcmp0 (attribute_names[i], name) == 0)
		{
			return attribute_values[i];
		}
	}

	return NULL;
}

static void
sessionmanager_xml_start_element (
	GMarkupParseContext  *context,
	const gchar          *element_name,
	const gchar         **attribute_names,
	const gchar         **attribute_values,
	gpointer              user_data,
	GError              **error
)
{
	SessionmanagerXmlParserData *data = user_data;

	(void) context;

	if (g_strcmp0 (element_name, "saved-sessions") == 0)
	{
		return;
	}

	if (g_strcmp0 (element_name, "session") == 0)
	{
		const gchar *name;

		name = sessionmanager_xml_find_attribute (
			attribute_names,
			attribute_values,
			"name"
		);

		if (name == NULL || name[0] == '\0')
		{
			g_set_error (
				error,
				G_MARKUP_ERROR,
				G_MARKUP_ERROR_INVALID_CONTENT,
				"session要素に有効なname属性がありません"
			);

			return;
		}

		data->current_session = sessionmanager_session_new (name);

		sessionmanager_session_store_add (
			data->store,
			data->current_session
		);

		return;
	}

	if (g_strcmp0 (element_name, "file") == 0)
	{
		const gchar *path;

		if (data->current_session == NULL)
		{
			g_set_error (
				error,
				G_MARKUP_ERROR,
				G_MARKUP_ERROR_INVALID_CONTENT,
				"file要素がsession要素の外にあります"
			);

			return;
		}

		path = sessionmanager_xml_find_attribute (
			attribute_names,
			attribute_values,
			"path"
		);

		if (path == NULL || path[0] == '\0')
		{
			g_set_error (
				error,
				G_MARKUP_ERROR,
				G_MARKUP_ERROR_INVALID_CONTENT,
				"file要素に有効なpath属性がありません"
			);

			return;
		}

		sessionmanager_session_add_file (
			data->current_session,
			path
		);

		return;
	}

	g_set_error (
		error,
		G_MARKUP_ERROR,
		G_MARKUP_ERROR_UNKNOWN_ELEMENT,
		"不明なXML要素です: %s",
		element_name
	);
}

static void
sessionmanager_xml_end_element (
	GMarkupParseContext *context,
	const gchar         *element_name,
	gpointer             user_data,
	GError             **error
)
{
	SessionmanagerXmlParserData *data = user_data;

	(void) context;
	(void) error;

	if (g_strcmp0 (element_name, "session") == 0)
	{
		data->current_session = NULL;
	}
}

gboolean
sessionmanager_xml_save (
	const SessionmanagerSessionStore *store,
	const gchar                    *path,
	GError                        **error
)
{
	GString *xml;
	guint i;

	g_return_val_if_fail (store != NULL, FALSE);
	g_return_val_if_fail (path != NULL, FALSE);

	xml = g_string_new (
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<saved-sessions>\n"
	);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionmanagerSession *session;
		gchar *escaped_name;
		guint j;

		session = g_ptr_array_index (
			store->sessions,
			i
		);

		escaped_name = g_markup_escape_text (
			session->name,
			-1
		);

		g_string_append_printf (
			xml,
			"  <session name=\"%s\">\n",
			escaped_name
		);

		g_free (escaped_name);

		for (j = 0; j < session->files->len; j++)
		{
			const gchar *uri;
			gchar *escaped_uri;

			uri = g_ptr_array_index (
				session->files,
				j
			);

			escaped_uri = g_markup_escape_text (
				uri,
				-1
			);

			g_string_append_printf (
				xml,
				"    <file path=\"%s\"/>\n",
				escaped_uri
			);

			g_free (escaped_uri);
		}

		g_string_append (
			xml,
			"  </session>\n"
		);
	}

	g_string_append (
		xml,
		"</saved-sessions>\n"
	);

	if (!g_file_set_contents (
		path,
		xml->str,
		(gssize) xml->len,
		error
	))
	{
		g_string_free (xml, TRUE);
		return FALSE;
	}

	g_string_free (xml, TRUE);

	return TRUE;
}

SessionmanagerSessionStore *
sessionmanager_xml_load (
	const gchar *path,
	GError     **error
)
{
	gchar *contents = NULL;
	gsize length = 0;
	SessionmanagerSessionStore *store;
	SessionmanagerXmlParserData data;
	GMarkupParser parser = {
		sessionmanager_xml_start_element,
		sessionmanager_xml_end_element,
		NULL,
		NULL,
		NULL
	};
	GMarkupParseContext *context;

	g_return_val_if_fail (path != NULL, NULL);

	if (!g_file_get_contents (
		path,
		&contents,
		&length,
		error
	))
	{
		return NULL;
	}

	store = sessionmanager_session_store_new ();

	data.store = store;
	data.current_session = NULL;

	context = g_markup_parse_context_new (
		&parser,
		G_MARKUP_TREAT_CDATA_AS_TEXT,
		&data,
		NULL
	);

	if (!g_markup_parse_context_parse (
		context,
		contents,
		(gssize) length,
		error
	) ||
	    !g_markup_parse_context_end_parse (
		context,
		error
	))
	{
		g_markup_parse_context_free (context);
		g_free (contents);
		sessionmanager_session_store_free (store);

		return NULL;
	}

	g_markup_parse_context_free (context);
	g_free (contents);

	return store;
}
