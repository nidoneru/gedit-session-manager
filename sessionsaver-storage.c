/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionsaver-storage.h"

#include "sessionsaver-xml.h"

#include <glib/gstdio.h>

static gchar *
sessionsaver_storage_get_path (void)
{
	gchar *directory;
	gchar *path;

	directory = g_build_filename (
		g_get_user_data_dir (),
		"gedit",
		"plugins",
		"sessionsaver",
		NULL
	);

	if (g_mkdir_with_parents (directory, 0700) != 0)
	{
		g_warning (
			"sessionsaver: 保存先ディレクトリを作成できませんでした: %s",
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

SessionsaverSessionStore *
sessionsaver_storage_load (
	GError **error
)
{
	SessionsaverSessionStore *store;
	gchar *path;

	path = sessionsaver_storage_get_path ();

	if (!g_file_test (path, G_FILE_TEST_EXISTS))
	{
		g_free (path);
		return NULL;
	}

	store = sessionsaver_xml_load (
		path,
		error
	);

	g_free (path);

	return store;
}

SessionsaverSessionStore *
sessionsaver_storage_load_or_new (
	GError **error
)
{
	SessionsaverSessionStore *store;
	gchar *path;

	path = sessionsaver_storage_get_path ();

	if (!g_file_test (path, G_FILE_TEST_EXISTS))
	{
		g_free (path);
		return sessionsaver_session_store_new ();
	}

	store = sessionsaver_xml_load (
		path,
		error
	);

	g_free (path);

	return store;
}

gboolean
sessionsaver_storage_save (
	const SessionsaverSessionStore *store,
	GError                        **error
)
{
	gboolean result;
	gchar *path;

	g_return_val_if_fail (store != NULL, FALSE);

	path = sessionsaver_storage_get_path ();

	result = sessionsaver_xml_save (
		store,
		path,
		error
	);

	g_free (path);

	return result;
}

SessionsaverSession *
sessionsaver_storage_get_previous (
	SessionsaverSessionStore *store
)
{
	g_return_val_if_fail (store != NULL, NULL);

	return sessionsaver_session_store_find (
		store,
		SESSIONSAVER_PREVIOUS_SESSION_NAME
	);
}

void
sessionsaver_storage_replace_session (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
)
{
	g_return_if_fail (store != NULL);
	g_return_if_fail (session != NULL);

	sessionsaver_session_store_replace (
		store,
		session
	);
}
