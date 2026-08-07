/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <glib.h>

#include "sessionsaver-session.h"

G_BEGIN_DECLS

SessionsaverSessionStore *
sessionsaver_storage_load (
	GError **error
);

SessionsaverSessionStore *
sessionsaver_storage_load_or_new (
	GError **error
);

gboolean
sessionsaver_storage_save (
	const SessionsaverSessionStore *store,
	GError                        **error
);

SessionsaverSession *
sessionsaver_storage_get_previous (
	SessionsaverSessionStore *store
);

void
sessionsaver_storage_replace_session (
	SessionsaverSessionStore *store,
	SessionsaverSession      *session
);

G_END_DECLS
