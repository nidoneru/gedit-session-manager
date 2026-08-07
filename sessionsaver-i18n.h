/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef SESSIONSAVER_I18N_H
#define SESSIONSAVER_I18N_H

#include <glib.h>

G_BEGIN_DECLS

const gchar *
sessionsaver_translate (
	const gchar *english,
	const gchar *japanese
);

gchar *
sessionsaver_format_restore_session (
	const gchar *session_name
);

gchar *
sessionsaver_format_duplicate_session_name (
	const gchar *session_name
);

gchar *
sessionsaver_format_delete_session (
	const gchar *session_name
);

G_END_DECLS

#endif /* SESSIONSAVER_I18N_H */
