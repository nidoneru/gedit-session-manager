/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>

#include "sessionsaver-session.h"

G_BEGIN_DECLS

gboolean
sessionsaver_save_session (
	SessionsaverSession *session,
	GError             **error
);

gboolean
sessionsaver_restore_session (
	GeditWindow *window,
	const gchar *name,
	GError     **error
);

gboolean
sessionsaver_delete_session (
	const gchar *name,
	GError     **error
);

gboolean
sessionsaver_rename_session (
	const gchar *old_name,
	const gchar *new_name,
	GError     **error
);

SessionsaverSessionStore *
sessionsaver_load_session_store (
	GError **error
);

void
sessionsaver_replace_with_session_async (
	GeditWindow         *window,
	const gchar         *name,
	GCancellable        *cancellable,
	GAsyncReadyCallback  callback,
	gpointer             user_data
);

gboolean
sessionsaver_replace_with_session_finish (
	GeditWindow  *window,
	GAsyncResult *result,
	GError      **error
);

G_END_DECLS
