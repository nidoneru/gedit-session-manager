/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <glib.h>

#include "sessionmanager-session.h"

G_BEGIN_DECLS

gboolean
sessionmanager_xml_save (
	const SessionmanagerSessionStore *store,
	const gchar                    *path,
	GError                        **error
);

SessionmanagerSessionStore *
sessionmanager_xml_load (
	const gchar *path,
	GError     **error
);

G_END_DECLS
