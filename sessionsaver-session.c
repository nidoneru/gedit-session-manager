/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionsaver-session.h"

SessionsaverSession *
sessionsaver_session_new (
	const gchar *name
)
{
	SessionsaverSession *session;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (name[0] != '\0', NULL);

	session = g_new0 (SessionsaverSession, 1);
	session->name = g_strdup (name);
	session->files = g_ptr_array_new_with_free_func (g_free);

	return session;
}

void
sessionsaver_session_free (
	SessionsaverSession *session
)
{
	if (session == NULL)
	{
		return;
	}

	g_free (session->name);
	g_ptr_array_unref (session->files);
	g_free (session);
}

void
sessionsaver_session_add_file (
	SessionsaverSession *session,
	const gchar         *uri
)
{
	g_return_if_fail (session != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (uri[0] != '\0');

	g_ptr_array_add (
		session->files,
		g_strdup (uri)
	);
}

SessionsaverSessionStore *
sessionsaver_session_store_new (void)
{
	SessionsaverSessionStore *store;

	store = g_new0 (SessionsaverSessionStore, 1);

	store->sessions = g_ptr_array_new_with_free_func (
		(GDestroyNotify) sessionsaver_session_free
	);

	return store;
}

void
sessionsaver_session_store_free (
	SessionsaverSessionStore *store
)
{
	if (store == NULL)
	{
		return;
	}

	g_ptr_array_unref (store->sessions);
	g_free (store);
}

void
sessionsaver_session_store_add (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
)
{
	g_return_if_fail (store != NULL);
	g_return_if_fail (session != NULL);

	g_ptr_array_add (
		store->sessions,
		session
	);
}

void
sessionsaver_session_store_replace (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
)
{
	guint i;

	g_return_if_fail (store != NULL);
	g_return_if_fail (session != NULL);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionsaverSession *current;

		current = g_ptr_array_index (
			store->sessions,
			i
		);

		if (g_ascii_strcasecmp (
			current->name,
			session->name
		) == 0)
		{
			g_ptr_array_remove_index (
				store->sessions,
				i
			);

			g_ptr_array_insert (
				store->sessions,
				i,
				session
			);

			return;
		}
	}

	g_ptr_array_add (
		store->sessions,
		session
	);
}

gboolean
sessionsaver_session_store_rename (
	SessionsaverSessionStore *store,
	const gchar              *old_name,
	const gchar              *new_name
)
{
	SessionsaverSession *session;
	SessionsaverSession *existing;

	g_return_val_if_fail (store != NULL, FALSE);
	g_return_val_if_fail (old_name != NULL, FALSE);
	g_return_val_if_fail (old_name[0] != '\0', FALSE);
	g_return_val_if_fail (new_name != NULL, FALSE);
	g_return_val_if_fail (new_name[0] != '\0', FALSE);

	session = sessionsaver_session_store_find (
		store,
		old_name
	);

	if (session == NULL)
	{
		return FALSE;
	}

	existing = sessionsaver_session_store_find (
		store,
		new_name
	);

	if (existing != NULL &&
	    existing != session)
	{
		return FALSE;
	}

	g_free (
		session->name
	);

	session->name = g_strdup (
		new_name
	);

	return TRUE;
}

gboolean
sessionsaver_session_store_remove (
	SessionsaverSessionStore *store,
	const gchar              *name
)
{
	guint i;

	g_return_val_if_fail (store != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (name[0] != '\0', FALSE);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionsaverSession *session;

		session = g_ptr_array_index (
			store->sessions,
			i
		);

		if (g_ascii_strcasecmp (
			session->name,
			name
		) == 0)
		{
			g_ptr_array_remove_index (
				store->sessions,
				i
			);

			return TRUE;
		}
	}

	return FALSE;
}

SessionsaverSession *
sessionsaver_session_store_find (
	SessionsaverSessionStore *store,
	const gchar              *name
)
{
	guint i;

	g_return_val_if_fail (store != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionsaverSession *session;

		session = g_ptr_array_index (
			store->sessions,
			i
		);

		if (g_ascii_strcasecmp (
			session->name,
			name
		) == 0)
		{
			return session;
		}
	}

	return NULL;
}

guint
sessionsaver_session_store_get_n_sessions (
	SessionsaverSessionStore *store
)
{
	g_return_val_if_fail (store != NULL, 0);

	return store->sessions->len;
}

SessionsaverSession *
sessionsaver_session_store_get_nth (
	SessionsaverSessionStore *store,
	guint                     index
)
{
	g_return_val_if_fail (store != NULL, NULL);

	if (index >= store->sessions->len)
	{
		return NULL;
	}

	return g_ptr_array_index (
		store->sessions,
		index
	);
}

