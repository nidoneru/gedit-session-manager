/* SPDX-License-Identifier: MIT */

#ifndef SESSIONMANAGER_I18N_H
#define SESSIONMANAGER_I18N_H

#include <glib.h>

G_BEGIN_DECLS

const gchar *
sessionmanager_translate (
	const gchar *english,
	const gchar *japanese
);

gchar *
sessionmanager_format_restore_session (
	const gchar *session_name
);

gchar *
sessionmanager_format_duplicate_session_name (
	const gchar *session_name
);

gchar *
sessionmanager_format_delete_session (
	const gchar *session_name
);

G_END_DECLS

#endif /* SESSIONMANAGER_I18N_H */
