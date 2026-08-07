/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define SESSIONSAVER_PREVIOUS_SESSION_NAME "Previous Session"

typedef struct _SessionsaverSession SessionsaverSession;
typedef struct _SessionsaverSessionStore SessionsaverSessionStore;

struct _SessionsaverSession
{
	gchar *name;
	GPtrArray *files;
};

struct _SessionsaverSessionStore
{
	GPtrArray *sessions;
};

SessionsaverSession *
sessionsaver_session_new (
	const gchar *name
);

void
sessionsaver_session_free (
	SessionsaverSession *session
);

void
sessionsaver_session_add_file (
	SessionsaverSession *session,
	const gchar         *uri
);

SessionsaverSessionStore *
sessionsaver_session_store_new (void);

void
sessionsaver_session_store_free (
	SessionsaverSessionStore *store
);

void
sessionsaver_session_store_add (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
);

void
sessionsaver_session_store_replace (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
);

SessionsaverSession *
sessionsaver_session_store_find (
	SessionsaverSessionStore *store,
	const gchar              *name
);

gboolean
sessionsaver_session_store_remove (
	SessionsaverSessionStore *store,
	const gchar              *name
);

gboolean
sessionsaver_session_store_rename (
	SessionsaverSessionStore *store,
	const gchar              *old_name,
	const gchar              *new_name
);

guint
sessionsaver_session_store_get_n_sessions (
	SessionsaverSessionStore *store
);

SessionsaverSession *
sessionsaver_session_store_get_nth (
	SessionsaverSessionStore *store,
	guint                     index
);

G_END_DECLS
