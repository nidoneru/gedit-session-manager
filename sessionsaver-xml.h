/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <glib.h>

#include "sessionsaver-session.h"

G_BEGIN_DECLS

gboolean
sessionsaver_xml_save (
	const SessionsaverSessionStore *store,
	const gchar                    *path,
	GError                        **error
);

SessionsaverSessionStore *
sessionsaver_xml_load (
	const gchar *path,
	GError     **error
);

G_END_DECLS
