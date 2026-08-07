/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionmanager-storage.h"

#include "sessionmanager-xml.h"

#include <glib/gstdio.h>

static gchar *
sessionmanager_storage_get_path (void)
{
	gchar *directory;
	gchar *path;

	directory = g_build_filename (
		g_get_user_data_dir (),
		"gedit",
		"plugins",
		"sessionmanager",
		NULL
	);

	if (g_mkdir_with_parents (directory, 0700) != 0)
	{
		g_warning (
			"sessionmanager: 保存先ディレクトリを作成できませんでした: %s",
			directory
		);
	}

	path = g_build_filename (
		directory,
		"saved-sessions.xml",
		NULL
	);

	g_free (directory);

	return path;
}

SessionmanagerSessionStore *
sessionmanager_storage_load (
	GError **error
)
{
	SessionmanagerSessionStore *store;
	gchar *path;

	path = sessionmanager_storage_get_path ();

	if (!g_file_test (path, G_FILE_TEST_EXISTS))
	{
		g_free (path);
		return NULL;
	}

	store = sessionmanager_xml_load (
		path,
		error
	);

	g_free (path);

	return store;
}

SessionmanagerSessionStore *
sessionmanager_storage_load_or_new (
	GError **error
)
{
	SessionmanagerSessionStore *store;
	gchar *path;

	path = sessionmanager_storage_get_path ();

	if (!g_file_test (path, G_FILE_TEST_EXISTS))
	{
		g_free (path);
		return sessionmanager_session_store_new ();
	}

	store = sessionmanager_xml_load (
		path,
		error
	);

	g_free (path);

	return store;
}

gboolean
sessionmanager_storage_save (
	const SessionmanagerSessionStore *store,
	GError                        **error
)
{
	gboolean result;
	gchar *path;

	g_return_val_if_fail (store != NULL, FALSE);

	path = sessionmanager_storage_get_path ();

	result = sessionmanager_xml_save (
		store,
		path,
		error
	);

	g_free (path);

	return result;
}

SessionmanagerSession *
sessionmanager_storage_get_previous (
	SessionmanagerSessionStore *store
)
{
	g_return_val_if_fail (store != NULL, NULL);

	return sessionmanager_session_store_find (
		store,
		SESSIONMANAGER_PREVIOUS_SESSION_NAME
	);
}

void
sessionmanager_storage_replace_session (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
)
{
	g_return_if_fail (store != NULL);
	g_return_if_fail (session != NULL);

	sessionmanager_session_store_replace (
		store,
		session
	);
}
