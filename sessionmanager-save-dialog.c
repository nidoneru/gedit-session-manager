/* SPDX-License-Identifier: MIT */

#include "sessionmanager-save-dialog.h"

#include "sessionmanager-service.h"
#include "sessionmanager-session.h"
#include "sessionmanager-window.h"

#include <glib/gi18n.h>
#include "sessionmanager-i18n.h"

struct _SessionmanagerSaveDialog
{
	GeditWindow *window;
	GtkWidget *widget;
	GtkWidget *combo;
	GtkWidget *entry;

	SessionmanagerSaveRefreshFunc refresh_func;
	gpointer user_data;
};

static void
sessionmanager_save_dialog_show_message (
	SessionmanagerSaveDialog *dialog,
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

static void
sessionmanager_save_dialog_entry_changed (
	GtkEditable *editable,
	gpointer     user_data
)
{
	SessionmanagerSaveDialog *dialog;
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
			SESSIONMANAGER_PREVIOUS_SESSION_NAME
		) != 0;

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
sessionmanager_save_dialog_response (
	GtkDialog *widget,
	gint       response_id,
	gpointer   user_data
)
{
	SessionmanagerSaveDialog *dialog;

	dialog = user_data;

	if (response_id == GTK_RESPONSE_OK)
	{
		gchar *name;
		SessionmanagerSession *session;
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
			SESSIONMANAGER_PREVIOUS_SESSION_NAME
		) == 0)
		{
			sessionmanager_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_ERROR,
				sessionmanager_translate ("This session name is reserved.", "このセッション名は予約されています。"),
				sessionmanager_translate ("Choose a different name.", "別の名前を指定してください。")
			);

			g_free (
				name
			);

			return;
		}

		session = sessionmanager_capture_window (
			dialog->window,
			name
		);

		if (session == NULL ||
		    session->files->len == 0)
		{
			sessionmanager_session_free (
				session
			);

			sessionmanager_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_INFO,
				sessionmanager_translate ("There are no saved documents to include.", "保存できる文書はありません。"),
				NULL
			);

			g_free (
				name
			);

			return;
		}

		if (!sessionmanager_save_session (
			session,
			&error
		))
		{
			sessionmanager_save_dialog_show_message (
				dialog,
				GTK_MESSAGE_ERROR,
				sessionmanager_translate ("The session could not be saved.", "セッションを保存できません。"),
				error != NULL
					? error->message
					: sessionmanager_translate ("Unknown error", "不明なエラー")
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
			SessionmanagerSaveRefreshFunc refresh_func;
			gpointer callback_data;

			refresh_func = dialog->refresh_func;
			callback_data = dialog->user_data;

			g_message (
				"sessionmanager: Saved session \"%s\"",
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
sessionmanager_save_dialog_free (
	SessionmanagerSaveDialog *dialog
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
	dialog->combo = NULL;
	dialog->entry = NULL;

	g_free (
		dialog
	);
}

SessionmanagerSaveDialog *
sessionmanager_save_dialog_new (
	GeditWindow                 *window,
	SessionmanagerSaveRefreshFunc  refresh_func,
	gpointer                     user_data
)
{
	SessionmanagerSaveDialog *dialog;
	GtkWidget *content_area;
	GtkWidget *box;
	GtkWidget *label;
	SessionmanagerSessionStore *store;
	GError *error = NULL;
	guint i;

	g_return_val_if_fail (
		GEDIT_IS_WINDOW (window),
		NULL
	);

	dialog = g_new0 (
		SessionmanagerSaveDialog,
		1
	);

	dialog->window = g_object_ref (
		window
	);

	dialog->refresh_func = refresh_func;
	dialog->user_data = user_data;

	dialog->widget = gtk_dialog_new_with_buttons (
		sessionmanager_translate ("Save Session", "セッションを保存"),
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL |
		GTK_DIALOG_DESTROY_WITH_PARENT,
		sessionmanager_translate ("_Cancel", "_キャンセル"),
		GTK_RESPONSE_CANCEL,
		sessionmanager_translate ("_Save", "_保存"),
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
		sessionmanager_translate ("Session name:", "セッション名:")
	);

	gtk_widget_set_halign (
		label,
		GTK_ALIGN_START
	);

	dialog->combo = gtk_combo_box_text_new_with_entry ();

	dialog->entry = gtk_bin_get_child (
		GTK_BIN (dialog->combo)
	);

	gtk_entry_set_activates_default (
		GTK_ENTRY (dialog->entry),
		TRUE
	);

	store = sessionmanager_load_session_store (
		&error
	);

	if (store != NULL)
	{
		for (i = 0;
		     i < sessionmanager_session_store_get_n_sessions (store);
		     i++)
		{
			SessionmanagerSession *session;

			session = sessionmanager_session_store_get_nth (
				store,
				i
			);

			if (session == NULL ||
			    g_ascii_strcasecmp (
				    session->name,
				    SESSIONMANAGER_PREVIOUS_SESSION_NAME
			    ) == 0)
			{
				continue;
			}

			gtk_combo_box_text_append_text (
				GTK_COMBO_BOX_TEXT (dialog->combo),
				session->name
			);
		}

		sessionmanager_session_store_free (
			store
		);
	}
	else
	{
		g_clear_error (
			&error
		);
	}

	gtk_box_pack_start (
		GTK_BOX (box),
		label,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (box),
		dialog->combo,
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
		G_CALLBACK (sessionmanager_save_dialog_entry_changed),
		dialog
	);

	g_signal_connect (
		dialog->widget,
		"response",
		G_CALLBACK (sessionmanager_save_dialog_response),
		dialog
	);

	g_object_set_data_full (
		G_OBJECT (dialog->widget),
		"sessionmanager-save-dialog",
		dialog,
		(GDestroyNotify)
			sessionmanager_save_dialog_free
	);

	return dialog;
}

GtkWidget *
sessionmanager_save_dialog_get_widget (
	SessionmanagerSaveDialog *dialog
)
{
	g_return_val_if_fail (
		dialog != NULL,
		NULL
	);

	return dialog->widget;
}

void
sessionmanager_save_dialog_present (
	SessionmanagerSaveDialog *dialog
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
