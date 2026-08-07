/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionsaver-save-dialog.h"

#include "sessionsaver-service.h"
#include "sessionsaver-session.h"
#include "sessionsaver-window.h"

#include <glib/gi18n.h>
#include "sessionsaver-i18n.h"

struct _SessionsaverSaveDialog
{
	GeditWindow *window;
	GtkWidget *widget;
	GtkWidget *entry;

	SessionsaverSaveRefreshFunc refresh_func;
	gpointer user_data;
};

static void
sessionsaver_save_dialog_show_message (
	SessionsaverSaveDialog *dialog,
	GtkMessageType          type,
	const gchar            *primary_text,
	const gchar            *secondary_text
)
{
	GtkWidget *message_dialog;

	message_dialog = gtk_message_dialog_new (
		GTK_WINDOW (dialog->widget),
		GTK_DIALOG_MODAL |
		GTK_DIALOG_DESTROY_WITH_PARENT,
		type,
		GTK_BUTTONS_CLOSE,
		"%s",
		primary_text
	);

	if (secondary_text != NULL)
	{
		gtk_message_dialog_format_secondary_text (
			GTK_MESSAGE_DIALOG (message_dialog),
			"%s",
			secondary_text
		);
	}

	gtk_dialog_run (
		GTK_DIALOG (message_dialog)
	);

	gtk_widget_destroy (
		message_dialog
	);
}

static gboolean
sessionsaver_save_dialog_name_exists (
	const gchar *name
)
{
	SessionsaverSessionStore *store;
	gboolean exists = FALSE;
	GError *error = NULL;

	store = sessionsaver_load_session_store (
		&error
	);

	if (store == NULL)
	{
		g_clear_error (
			&error
		);

		return FALSE;
	}

	exists =
		sessionsaver_session_store_find (
			store,
			name
		) != NULL;

	sessionsaver_session_store_free (
		store
	);

	return exists;
}

static void
sessionsaver_save_dialog_entry_changed (
	GtkEditable *editable,
	gpointer     user_data
)
{
	SessionsaverSaveDialog *dialog;
	gchar *name;
	gboolean valid;

	dialog = user_data;

	name = g_strdup (
		gtk_entry_get_text (
			GTK_ENTRY (editable)
		)
	);

	g_strstrip (
		name
	);

	valid =
		name[0] != '\0' &&
		g_ascii_strcasecmp (
			name,
			SESSIONSAVER_PREVIOUS_SESSION_NAME
		) != 0 &&
		!sessionsaver_save_dialog_name_exists (
			name
		);

	gtk_dialog_set_response_sensitive (
		GTK_DIALOG (dialog->widget),
		GTK_RESPONSE_OK,
		valid
	);

	g_free (
		name
	);
}

static void
sessionsaver_save_dialog_response (
	GtkDialog *widget,
	gint       response_id,
	gpointer   user_data
)
{
	SessionsaverSaveDialog *dialog;

	dialog = user_data;

	if (response_id == GTK_RESPONSE_OK)
	{
		gchar *name;
		SessionsaverSession *session;
		GError *error = NULL;

		name = g_strdup (
			gtk_entry_get_text (
				GTK_ENTRY (dialog->entry)
			)
		);

		g_strstrip (
			name
		);

		/*
		 * Defensive check.
		 * The Save button is already disabled for this reserved name.
		 */
		if (g_ascii_strcasecmp (
			name,
			SESSIONSAVER_PREVIOUS_SESSION_NAME
		) == 0)
		{
			sessionsaver_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_ERROR,
				sessionsaver_translate ("This session name is reserved.", "このセッション名は予約されています。"),
				sessionsaver_translate ("Choose a different name.", "別の名前を指定してください。")
			);

			g_free (
				name
			);

			return;
		}

		session = sessionsaver_capture_window (
			dialog->window,
			name
		);

		if (session == NULL ||
		    session->files->len == 0)
		{
			sessionsaver_session_free (
				session
			);

			sessionsaver_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_INFO,
				sessionsaver_translate ("There are no saved documents to include.", "保存できる文書はありません。"),
				NULL
			);

			g_free (
				name
			);

			return;
		}

		if (!sessionsaver_save_session (
			session,
			&error
		))
		{
			sessionsaver_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_ERROR,
				sessionsaver_translate ("The session could not be saved.", "セッションを保存できません。"),
				error != NULL
					? error->message
					: sessionsaver_translate ("Unknown error", "不明なエラー")
			);

			g_clear_error (
				&error
			);

			g_free (
				name
			);

			return;
		}

		{
			SessionsaverSaveRefreshFunc refresh_func;
			gpointer callback_data;

			refresh_func = dialog->refresh_func;
			callback_data = dialog->user_data;

			g_message (
				"sessionsaver: Saved session \"%s\"",
				name
			);

			gtk_widget_destroy (
				GTK_WIDGET (widget)
			);

			if (refresh_func != NULL)
			{
				refresh_func (
					callback_data
				);
			}
		}

		g_free (
			name
		);

		return;
	}

	gtk_widget_destroy (
		GTK_WIDGET (widget)
	);
}

static void
sessionsaver_save_dialog_free (
	SessionsaverSaveDialog *dialog
)
{
	if (dialog == NULL)
	{
		return;
	}

	g_clear_object (
		&dialog->window
	);

	dialog->widget = NULL;
	dialog->entry = NULL;

	g_free (
		dialog
	);
}

SessionsaverSaveDialog *
sessionsaver_save_dialog_new (
	GeditWindow                 *window,
	SessionsaverSaveRefreshFunc  refresh_func,
	gpointer                     user_data
)
{
	SessionsaverSaveDialog *dialog;
	GtkWidget *content_area;
	GtkWidget *box;
	GtkWidget *label;

	g_return_val_if_fail (
		GEDIT_IS_WINDOW (window),
		NULL
	);

	dialog = g_new0 (
		SessionsaverSaveDialog,
		1
	);

	dialog->window = g_object_ref (
		window
	);

	dialog->refresh_func = refresh_func;
	dialog->user_data = user_data;

	dialog->widget = gtk_dialog_new_with_buttons (
		sessionsaver_translate ("Save Session", "セッションを保存"),
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL |
		GTK_DIALOG_DESTROY_WITH_PARENT,
		sessionsaver_translate ("_Cancel", "_キャンセル"),
		GTK_RESPONSE_CANCEL,
		sessionsaver_translate ("_Save", "_保存"),
		GTK_RESPONSE_OK,
		NULL
	);

	gtk_dialog_set_default_response (
		GTK_DIALOG (dialog->widget),
		GTK_RESPONSE_OK
	);

	gtk_dialog_set_response_sensitive (
		GTK_DIALOG (dialog->widget),
		GTK_RESPONSE_OK,
		FALSE
	);

	content_area = gtk_dialog_get_content_area (
		GTK_DIALOG (dialog->widget)
	);

	box = gtk_box_new (
		GTK_ORIENTATION_VERTICAL,
		8
	);

	gtk_container_set_border_width (
		GTK_CONTAINER (box),
		12
	);

	label = gtk_label_new (
		sessionsaver_translate ("Session name:", "セッション名:")
	);

	gtk_widget_set_halign (
		label,
		GTK_ALIGN_START
	);

	dialog->entry = gtk_entry_new ();

	gtk_entry_set_activates_default (
		GTK_ENTRY (dialog->entry),
		TRUE
	);

	gtk_box_pack_start (
		GTK_BOX (box),
		label,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (box),
		dialog->entry,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (content_area),
		box,
		TRUE,
		TRUE,
		0
	);

	g_signal_connect (
		dialog->entry,
		"changed",
		G_CALLBACK (sessionsaver_save_dialog_entry_changed),
		dialog
	);

	g_signal_connect (
		dialog->widget,
		"response",
		G_CALLBACK (sessionsaver_save_dialog_response),
		dialog
	);

	g_object_set_data_full (
		G_OBJECT (dialog->widget),
		"sessionsaver-save-dialog",
		dialog,
		(GDestroyNotify)
			sessionsaver_save_dialog_free
	);

	return dialog;
}

GtkWidget *
sessionsaver_save_dialog_get_widget (
	SessionsaverSaveDialog *dialog
)
{
	g_return_val_if_fail (
		dialog != NULL,
		NULL
	);

	return dialog->widget;
}

void
sessionsaver_save_dialog_present (
	SessionsaverSaveDialog *dialog
)
{
	g_return_if_fail (
		dialog != NULL
	);

	g_return_if_fail (
		GTK_IS_WINDOW (dialog->widget)
	);

	gtk_window_present (
		GTK_WINDOW (dialog->widget)
	);
}
