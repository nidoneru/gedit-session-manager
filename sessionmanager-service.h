/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>

#include "sessionmanager-session.h"

G_BEGIN_DECLS

gboolean
sessionmanager_save_session (
	SessionmanagerSession *session,
	GError             **error
);

gboolean
sessionmanager_restore_session (
	GeditWindow *window,
	const gchar *name,
	GError     **error
);

gboolean
sessionmanager_delete_session (
	const gchar *name,
	GError     **error
);

gboolean
sessionmanager_rename_session (
	const gchar *old_name,
	const gchar *new_name,
	GError     **error
);

SessionmanagerSessionStore *
sessionmanager_load_session_store (
	GError **error
);

void
sessionmanager_replace_with_session_async (
	GeditWindow         *window,
	const gchar         *name,
	GCancellable        *cancellable,
	GAsyncReadyCallback  callback,
	gpointer             user_data
);

gboolean
sessionmanager_replace_with_session_finish (
	GeditWindow  *window,
	GAsyncResult *result,
	GError      **error
);

G_END_DECLS
