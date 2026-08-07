/* SPDX-License-Identifier: MIT */

#pragma once

#include <glib.h>

#include "sessionmanager-session.h"

G_BEGIN_DECLS

SessionmanagerSessionStore *
sessionmanager_storage_load (
	GError **error
);

SessionmanagerSessionStore *
sessionmanager_storage_load_or_new (
	GError **error
);

gboolean
sessionmanager_storage_save (
	const SessionmanagerSessionStore *store,
	GError                        **error
);

SessionmanagerSession *
sessionmanager_storage_get_previous (
	SessionmanagerSessionStore *store
);

void
sessionmanager_storage_replace_session (
	SessionmanagerSessionStore *store,
	SessionmanagerSession      *session
);

G_END_DECLS
