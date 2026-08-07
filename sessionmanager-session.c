/* SPDX-License-Identifier: MIT */

#include "sessionmanager-session.h"

SessionmanagerSession *
sessionmanager_session_new (
	const gchar *name
)
{
	SessionmanagerSession *session;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (name[0] != '\0', NULL);

	session = g_new0 (SessionmanagerSession, 1);
	session->name = g_strdup (name);
	session->files = g_ptr_array_new_with_free_func (g_free);

	return session;
}

void
sessionmanager_session_free (
	SessionmanagerSession *session
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
sessionmanager_session_add_file (
	SessionmanagerSession *session,
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

SessionmanagerSessionStore *
sessionmanager_session_store_new (void)
{
	SessionmanagerSessionStore *store;

	store = g_new0 (SessionmanagerSessionStore, 1);

	store->sessions = g_ptr_array_new_with_free_func (
		(GDestroyNotify) sessionmanager_session_free
	);

	return store;
}

void
sessionmanager_session_store_free (
	SessionmanagerSessionStore *store
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
sessionmanager_session_store_add (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
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
sessionmanager_session_store_replace (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
)
{
	guint i;

	g_return_if_fail (store != NULL);
	g_return_if_fail (session != NULL);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionmanagerSession *current;

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
sessionmanager_session_store_rename (
	SessionmanagerSessionStore *store,
	const gchar              *old_name,
	const gchar              *new_name
)
{
	SessionmanagerSession *session;
	SessionmanagerSession *existing;

	g_return_val_if_fail (store != NULL, FALSE);
	g_return_val_if_fail (old_name != NULL, FALSE);
	g_return_val_if_fail (old_name[0] != '\0', FALSE);
	g_return_val_if_fail (new_name != NULL, FALSE);
	g_return_val_if_fail (new_name[0] != '\0', FALSE);

	session = sessionmanager_session_store_find (
		store,
		old_name
	);

	if (session == NULL)
	{
		return FALSE;
	}

	existing = sessionmanager_session_store_find (
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
sessionmanager_session_store_remove (
	SessionmanagerSessionStore *store,
	const gchar              *name
)
{
	guint i;

	g_return_val_if_fail (store != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (name[0] != '\0', FALSE);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionmanagerSession *session;

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

SessionmanagerSession *
sessionmanager_session_store_find (
	SessionmanagerSessionStore *store,
	const gchar              *name
)
{
	guint i;

	g_return_val_if_fail (store != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	for (i = 0; i < store->sessions->len; i++)
	{
		SessionmanagerSession *session;

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
sessionmanager_session_store_get_n_sessions (
	SessionmanagerSessionStore *store
)
{
	g_return_val_if_fail (store != NULL, 0);

	return store->sessions->len;
}

SessionmanagerSession *
sessionmanager_session_store_get_nth (
	SessionmanagerSessionStore *store,
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

