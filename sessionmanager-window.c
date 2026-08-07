/* SPDX-License-Identifier: MIT */

#include "sessionmanager-window.h"

#include <gedit/gedit-commands.h>
#include <gedit/gedit-document.h>
#include <gtksourceview/gtksource.h>

SessionmanagerSession *
sessionmanager_capture_window (
	GeditWindow *window,
	const gchar *name
)
{
	SessionmanagerSession *session;
	GList *documents;
	GList *item;

	g_return_val_if_fail (GEDIT_IS_WINDOW (window), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (name[0] != '\0', NULL);

	session = sessionmanager_session_new (name);
	documents = gedit_window_get_documents (window);

	for (item = documents; item != NULL; item = item->next)
	{
		GeditDocument *document;
		GtkSourceFile *source_file;
		GFile *location;
		gchar *uri;

		document = GEDIT_DOCUMENT (item->data);
		source_file = gedit_document_get_file (document);
		location = gtk_source_file_get_location (source_file);

		/*
		 * 未保存の無題ドキュメントは、セッションファイルには
		 * 記録できないため除外する。
		 */
		if (location == NULL)
		{
			continue;
		}

		uri = g_file_get_uri (location);

		if (uri != NULL)
		{
			sessionmanager_session_add_file (
				session,
				uri
			);

			g_free (uri);
		}
	}

	g_list_free (documents);

	return session;
}

gboolean
sessionmanager_restore_window (
	GeditWindow               *window,
	const SessionmanagerSession *session
)
{
	GSList *locations = NULL;
	GSList *loaded;
	guint i;

	g_return_val_if_fail (GEDIT_IS_WINDOW (window), FALSE);
	g_return_val_if_fail (session != NULL, FALSE);

	for (i = 0; i < session->files->len; i++)
	{
		const gchar *uri;

		uri = g_ptr_array_index (
			session->files,
			i
		);

		locations = g_slist_prepend (
			locations,
			g_file_new_for_uri (uri)
		);
	}

	if (locations == NULL)
	{
		return FALSE;
	}

	locations = g_slist_reverse (locations);

	loaded = gedit_commands_load_locations (
		window,
		locations,
		NULL,
		0,
		0
	);

	g_slist_free_full (
		locations,
		g_object_unref
	);

	if (loaded == NULL)
	{
		return FALSE;
	}

	g_slist_free (loaded);

	return TRUE;
}
