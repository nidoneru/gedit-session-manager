/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionsaver-i18n.h"

static gboolean
sessionsaver_language_is_japanese (
	const gchar *language
)
{
	if (language == NULL ||
	    language[0] != 'j' ||
	    language[1] != 'a')
	{
		return FALSE;
	}

	return language[2] == '\0' ||
	       language[2] == '_' ||
	       language[2] == '-' ||
	       language[2] == '.' ||
	       language[2] == '@';
}

const gchar *
sessionsaver_translate (
	const gchar *english,
	const gchar *japanese
)
{
	const gchar * const *languages;
	guint i;

	g_return_val_if_fail (english != NULL, NULL);
	g_return_val_if_fail (japanese != NULL, english);

	languages = g_get_language_names ();

	for (i = 0; languages[i] != NULL; i++)
	{
		if (sessionsaver_language_is_japanese (
			languages[i]
		))
		{
			return japanese;
		}
	}

	return english;
}

gchar *
sessionsaver_format_restore_session (
	const gchar *session_name
)
{
	g_return_val_if_fail (session_name != NULL, NULL);

	if (sessionsaver_translate ("en", "ja")[0] == 'j')
	{
		return g_strdup_printf (
			"セッション「%s」を復元",
			session_name
		);
	}

	return g_strdup_printf (
		"Restore Session \"%s\"",
		session_name
	);
}

gchar *
sessionsaver_format_delete_session (
	const gchar *session_name
)
{
	g_return_val_if_fail (session_name != NULL, NULL);

	if (sessionsaver_translate ("en", "ja")[0] == 'j')
	{
		return g_strdup_printf (
			"セッション「%s」を削除しますか？",
			session_name
		);
	}

	return g_strdup_printf (
		"Delete session \"%s\"?",
		session_name
	);
}


gchar *
sessionsaver_format_duplicate_session_name (
	const gchar *session_name
)
{
	g_return_val_if_fail (session_name != NULL, NULL);

	if (sessionsaver_translate ("en", "ja")[0] == 'j')
	{
		return g_strdup_printf (
			"同名のセッションが既に存在します: %s",
			session_name
		);
	}

	return g_strdup_printf (
		"A session with the same name already exists: %s",
		session_name
	);
}
