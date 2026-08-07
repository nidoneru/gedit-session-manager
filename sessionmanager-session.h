/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define SESSIONMANAGER_PREVIOUS_SESSION_NAME "Previous Session"

typedef struct _SessionmanagerSession SessionmanagerSession;
typedef struct _SessionmanagerSessionStore SessionmanagerSessionStore;

struct _SessionmanagerSession
{
	gchar *name;
	GPtrArray *files;
};

struct _SessionmanagerSessionStore
{
	GPtrArray *sessions;
};

SessionmanagerSession *
sessionmanager_session_new (
	const gchar *name
);

void
sessionmanager_session_free (
	SessionmanagerSession *session
);

void
sessionmanager_session_add_file (
	SessionmanagerSession *session,
	const gchar         *uri
);

SessionmanagerSessionStore *
sessionmanager_session_store_new (void);

void
sessionmanager_session_store_free (
	SessionmanagerSessionStore *store
);

void
sessionmanager_session_store_add (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
);

void
sessionmanager_session_store_replace (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
);

SessionmanagerSession *
sessionmanager_session_store_find (
	SessionmanagerSessionStore *store,
	const gchar              *name
);

gboolean
sessionmanager_session_store_remove (
	SessionmanagerSessionStore *store,
	const gchar              *name
);

gboolean
sessionmanager_session_store_rename (
	SessionmanagerSessionStore *store,
	const gchar              *old_name,
	const gchar              *new_name
);

guint
sessionmanager_session_store_get_n_sessions (
	SessionmanagerSessionStore *store
);

SessionmanagerSession *
sessionmanager_session_store_get_nth (
	SessionmanagerSessionStore *store,
	guint                     index
);

G_END_DECLS
