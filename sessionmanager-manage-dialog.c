/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "sessionmanager-manage-dialog.h"

#include "sessionmanager-service.h"

#include <glib/gi18n.h>
#include "sessionmanager-i18n.h"

struct _SessionmanagerManageDialog
{
	GeditWindow *window;
	GtkWidget *widget;
	GtkWidget *list_box;
	GtkWidget *restore_button;
	GtkWidget *rename_button;
	GtkWidget *delete_button;

	SessionmanagerManageRestoreFunc restore_func;
	SessionmanagerManageRefreshFunc refresh_func;
	gpointer user_data;
};

static GtkWidget *
sessionmanager_manage_create_session_row (
	const gchar *session_name
)
{
	GtkWidget *row;
	GtkWidget *label;

	g_return_val_if_fail (
		session_name != NULL,
		NULL
	);

	row = gtk_list_box_row_new ();
	label = gtk_label_new (session_name);

	gtk_widget_set_halign (
		label,
		GTK_ALIGN_START
	);

	gtk_label_set_ellipsize (
		GTK_LABEL (label),
		PANGO_ELLIPSIZE_END
	);

	gtk_label_set_max_width_chars (
		GTK_LABEL (label),
		32
	);

	gtk_widget_set_tooltip_text (
		label,
		session_name
	);

	gtk_widget_set_margin_top (label, 8);
	gtk_widget_set_margin_bottom (label, 8);
	gtk_widget_set_margin_start (label, 8);
	gtk_widget_set_margin_end (label, 8);

	gtk_container_add (
		GTK_CONTAINER (row),
		label
	);

	return row;
}

static gboolean
sessionmanager_manage_populate_list (
	SessionmanagerManageDialog *dialog,
	GError                  **error
)
{
	SessionmanagerSessionStore *store;
	guint i;
	guint named_session_count = 0;

	store = sessionmanager_load_session_store (
		error
	);

	if (store == NULL)
	{
		return FALSE;
	}

	for (i = 0;
	     i < sessionmanager_session_store_get_n_sessions (store);
	     i++)
	{
		SessionmanagerSession *session;
		GtkWidget *row;

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

		row = sessionmanager_manage_create_session_row (
			session->name
		);

		gtk_container_add (
			GTK_CONTAINER (dialog->list_box),
			row
		);

		named_session_count++;
	}

	sessionmanager_session_store_free (
		store
	);

	if (named_session_count == 0)
	{
		GtkWidget *label;

		label = gtk_label_new (
			sessionmanager_translate ("No saved sessions.", "保存済みのセッションはありません。")
		);

		gtk_widget_set_sensitive (
			label,
			FALSE
		);

		gtk_widget_set_margin_top (label, 12);
		gtk_widget_set_margin_bottom (label, 12);
		gtk_widget_set_margin_start (label, 12);
		gtk_widget_set_margin_end (label, 12);

		gtk_container_add (
			GTK_CONTAINER (dialog->list_box),
			label
		);
	}

	return TRUE;
}

static gchar *
sessionmanager_manage_get_selected_session_name (
	SessionmanagerManageDialog *dialog
)
{
	GtkListBoxRow *row;
	GtkWidget *child;

	row = gtk_list_box_get_selected_row (
		GTK_LIST_BOX (dialog->list_box)
	);

	if (row == NULL)
	{
		return NULL;
	}

	child = gtk_bin_get_child (
		GTK_BIN (row)
	);

	if (!GTK_IS_LABEL (child))
	{
		return NULL;
	}

	return g_strdup (
		gtk_label_get_text (
			GTK_LABEL (child)
		)
	);
}

static void
sessionmanager_manage_restore_clicked (
	GtkButton *button,
	gpointer   user_data
)
{
	SessionmanagerManageDialog *dialog;
	SessionmanagerManageRestoreFunc restore_func;
	GeditWindow *window;
	gpointer callback_data;
	GtkWidget *widget;
	gchar *session_name;

	(void) button;

	dialog = user_data;

	session_name =
		sessionmanager_manage_get_selected_session_name (
			dialog
		);

	if (session_name == NULL)
	{
		return;
	}

	restore_func = dialog->restore_func;
	callback_data = dialog->user_data;
	widget = dialog->widget;
	window = g_object_ref (
		dialog->window
	);

	gtk_widget_destroy (
		widget
	);

	if (restore_func != NULL)
	{
		restore_func (
			window,
			session_name,
			callback_data
		);
	}

	g_object_unref (
		window
	);

	g_free (
		session_name
	);
}

static void
sessionmanager_manage_show_message (
	SessionmanagerManageDialog *dialog,
	GtkMessageType            type,
	const gchar              *primary_text,
	const gchar              *secondary_text
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
sessionmanager_manage_delete_clicked (
	GtkButton *button,
	gpointer   user_data
)
{
	SessionmanagerManageDialog *dialog;
	GtkWidget *confirm_dialog;
	GtkListBoxRow *row;
	gchar *session_name;
	gint response;
	GError *error = NULL;

	(void) button;

	dialog = user_data;

	session_name =
		sessionmanager_manage_get_selected_session_name (
			dialog
		);

	if (session_name == NULL)
	{
		return;
	}

	{
		gchar *message;

		message = sessionmanager_format_delete_session (
			session_name
		);

		confirm_dialog = gtk_message_dialog_new (
			GTK_WINDOW (dialog->widget),
			GTK_DIALOG_MODAL |
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_NONE,
			"%s",
			message
		);

		g_free (
			message
		);
	}

	gtk_message_dialog_format_secondary_text (
		GTK_MESSAGE_DIALOG (confirm_dialog),
		"%s",
		sessionmanager_translate (
			"This action cannot be undone.",
			"この操作は元に戻せません。"
		)
	);

	gtk_dialog_add_buttons (
		GTK_DIALOG (confirm_dialog),
		sessionmanager_translate ("Cancel", "キャンセル"),
		GTK_RESPONSE_CANCEL,
		sessionmanager_translate ("Delete", "削除"),
		GTK_RESPONSE_DELETE_EVENT,
		NULL
	);

	gtk_dialog_set_default_response (
		GTK_DIALOG (confirm_dialog),
		GTK_RESPONSE_CANCEL
	);

	response = gtk_dialog_run (
		GTK_DIALOG (confirm_dialog)
	);

	gtk_widget_destroy (
		confirm_dialog
	);

	if (response != GTK_RESPONSE_DELETE_EVENT)
	{
		g_free (
			session_name
		);

		return;
	}

	if (!sessionmanager_delete_session (
		session_name,
		&error
	))
	{
		sessionmanager_manage_show_message (
			dialog,
			GTK_MESSAGE_ERROR,
			sessionmanager_translate ("The session could not be deleted.", "セッションを削除できません。"),
			error != NULL
				? error->message
				: NULL
		);

		g_clear_error (
			&error
		);

		g_free (
			session_name
		);

		return;
	}

	row = gtk_list_box_get_selected_row (
		GTK_LIST_BOX (dialog->list_box)
	);

	if (row != NULL)
	{
		gtk_widget_destroy (
			GTK_WIDGET (row)
		);
	}

	gtk_widget_set_sensitive (
		dialog->restore_button,
		FALSE
	);

	gtk_widget_set_sensitive (
		dialog->rename_button,
		FALSE
	);

	gtk_widget_set_sensitive (
		dialog->delete_button,
		FALSE
	);

	if (dialog->refresh_func != NULL)
	{
		dialog->refresh_func (
			dialog->user_data
		);
	}

	g_free (
		session_name
	);
}

static void
sessionmanager_manage_rename_clicked (
	GtkButton *button,
	gpointer   user_data
)
{
	SessionmanagerManageDialog *dialog;
	GtkWidget *rename_dialog;
	GtkWidget *content_area;
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *entry;
	GtkListBoxRow *row;
	GtkWidget *row_label;
	gchar *old_name;
	gchar *new_name;
	gint response;
	GError *error = NULL;

	(void) button;

	dialog = user_data;

	old_name =
		sessionmanager_manage_get_selected_session_name (
			dialog
		);

	if (old_name == NULL)
	{
		return;
	}

	rename_dialog = gtk_dialog_new_with_buttons (
		sessionmanager_translate ("Rename Session", "セッション名を変更"),
		GTK_WINDOW (dialog->widget),
		GTK_DIALOG_MODAL |
		GTK_DIALOG_DESTROY_WITH_PARENT,
		sessionmanager_translate ("Cancel", "キャンセル"),
		GTK_RESPONSE_CANCEL,
		sessionmanager_translate ("Rename", "名前を変更"),
		GTK_RESPONSE_OK,
		NULL
	);

	gtk_dialog_set_default_response (
		GTK_DIALOG (rename_dialog),
		GTK_RESPONSE_OK
	);

	content_area = gtk_dialog_get_content_area (
		GTK_DIALOG (rename_dialog)
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
		sessionmanager_translate ("New session name:", "新しいセッション名:")
	);

	gtk_widget_set_halign (
		label,
		GTK_ALIGN_START
	);

	entry = gtk_entry_new ();

	gtk_entry_set_text (
		GTK_ENTRY (entry),
		old_name
	);

	gtk_editable_select_region (
		GTK_EDITABLE (entry),
		0,
		-1
	);

	gtk_entry_set_activates_default (
		GTK_ENTRY (entry),
		TRUE
	);

	gtk_box_pack_start (
		GTK_BOX (box),
		entry,
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

	gtk_widget_show_all (
		rename_dialog
	);

	response = gtk_dialog_run (
		GTK_DIALOG (rename_dialog)
	);

	if (response != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (
			rename_dialog
		);

		g_free (
			old_name
		);

		return;
	}

	new_name = g_strdup (
		gtk_entry_get_text (
			GTK_ENTRY (entry)
		)
	);

	g_strstrip (
		new_name
	);

	gtk_widget_destroy (
		rename_dialog
	);

	if (new_name[0] == '\0' ||
	    g_strcmp0 (
		    old_name,
		    new_name
	    ) == 0)
	{
		g_free (
			new_name
		);

		g_free (
			old_name
		);

		return;
	}

	if (!sessionmanager_rename_session (
		old_name,
		new_name,
		&error
	))
	{
		sessionmanager_manage_show_message (
			dialog,
			GTK_MESSAGE_ERROR,
			sessionmanager_translate ("The session could not be renamed.", "セッション名を変更できません。"),
			error != NULL
				? error->message
				: NULL
		);

		g_clear_error (
			&error
		);

		g_free (
			new_name
		);

		g_free (
			old_name
		);

		return;
	}

	row = gtk_list_box_get_selected_row (
		GTK_LIST_BOX (dialog->list_box)
	);

	if (row != NULL)
	{
		row_label = gtk_bin_get_child (
			GTK_BIN (row)
		);

		if (GTK_IS_LABEL (row_label))
		{
			gtk_label_set_text (
				GTK_LABEL (row_label),
				new_name
			);
		}
	}

	if (dialog->refresh_func != NULL)
	{
		dialog->refresh_func (
			dialog->user_data
		);
	}

	g_free (
		new_name
	);

	g_free (
		old_name
	);
}

static gboolean
sessionmanager_manage_button_pressed (
	GtkWidget      *widget,
	GdkEventButton *event,
	gpointer        user_data
)
{
	SessionmanagerManageDialog *dialog;
	GtkListBoxRow *row;

	if (event->type != GDK_2BUTTON_PRESS ||
	    event->button != GDK_BUTTON_PRIMARY)
	{
		return GDK_EVENT_PROPAGATE;
	}

	dialog = user_data;

	row = gtk_list_box_get_row_at_y (
		GTK_LIST_BOX (widget),
		(gint) event->y
	);

	if (row == NULL)
	{
		return GDK_EVENT_PROPAGATE;
	}

	gtk_list_box_select_row (
		GTK_LIST_BOX (widget),
		row
	);

	sessionmanager_manage_restore_clicked (
		NULL,
		dialog
	);

	return GDK_EVENT_STOP;
}

static gboolean
sessionmanager_manage_key_pressed (
	GtkWidget   *widget,
	GdkEventKey *event,
	gpointer     user_data
)
{
	SessionmanagerManageDialog *dialog;

	(void) widget;

	dialog = user_data;

	if (sessionmanager_manage_get_selected_session_name (
		dialog
	) == NULL)
	{
		return GDK_EVENT_PROPAGATE;
	}

	if (event->keyval == GDK_KEY_F2)
	{
		sessionmanager_manage_rename_clicked (
			NULL,
			dialog
		);

		return GDK_EVENT_STOP;
	}

	if (event->keyval == GDK_KEY_Return ||
	    event->keyval == GDK_KEY_KP_Enter)
	{
		sessionmanager_manage_restore_clicked (
			NULL,
			dialog
		);

		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static void
sessionmanager_manage_selection_changed (
	GtkListBox    *list_box,
	GtkListBoxRow *row,
	gpointer       user_data
)
{
	SessionmanagerManageDialog *dialog;
	gboolean has_selection;

	(void) list_box;

	dialog = user_data;
	has_selection = row != NULL;

	gtk_widget_set_sensitive (
		dialog->restore_button,
		has_selection
	);

	gtk_widget_set_sensitive (
		dialog->rename_button,
		has_selection
	);

	gtk_widget_set_sensitive (
		dialog->delete_button,
		has_selection
	);
}

static void
sessionmanager_manage_dialog_free (
	SessionmanagerManageDialog *dialog
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

	g_free (
		dialog
	);
}

SessionmanagerManageDialog *
sessionmanager_manage_dialog_new (
	GeditWindow                    *window,
	SessionmanagerManageRestoreFunc   restore_func,
	SessionmanagerManageRefreshFunc   refresh_func,
	gpointer                        user_data
)
{
	SessionmanagerManageDialog *dialog;
	GError *error = NULL;
	GtkWidget *content_area;
	GtkWidget *box;
	GtkWidget *content_box;
	GtkWidget *button_box;
	GtkWidget *close_button;

	g_return_val_if_fail (
		GEDIT_IS_WINDOW (window),
		NULL
	);

	dialog = g_new0 (
		SessionmanagerManageDialog,
		1
	);

	dialog->window = g_object_ref (
		window
	);

	dialog->restore_func = restore_func;
	dialog->refresh_func = refresh_func;
	dialog->user_data = user_data;

	dialog->widget = gtk_dialog_new_with_buttons (
		sessionmanager_translate ("Session Manager", "セッション管理"),
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL |
		GTK_DIALOG_DESTROY_WITH_PARENT,
		NULL,
		NULL
	);

	gtk_window_set_default_size (
		GTK_WINDOW (dialog->widget),
		420,
		300
	);

	content_area = gtk_dialog_get_content_area (
		GTK_DIALOG (dialog->widget)
	);

	box = gtk_box_new (
		GTK_ORIENTATION_VERTICAL,
		8
	);

	content_box = gtk_box_new (
		GTK_ORIENTATION_HORIZONTAL,
		12
	);

	button_box = gtk_box_new (
		GTK_ORIENTATION_VERTICAL,
		8
	);

	gtk_container_set_border_width (
		GTK_CONTAINER (box),
		12
	);

	dialog->list_box = gtk_list_box_new ();

	gtk_list_box_set_selection_mode (
		GTK_LIST_BOX (dialog->list_box),
		GTK_SELECTION_SINGLE
	);

	dialog->restore_button = gtk_button_new_with_mnemonic (
		sessionmanager_translate ("Restore", "復元")
	);

	dialog->rename_button = gtk_button_new_with_mnemonic (
		sessionmanager_translate ("Rename", "名前を変更")
	);

	dialog->delete_button = gtk_button_new_with_mnemonic (
		sessionmanager_translate ("Delete", "削除")
	);

	close_button = gtk_button_new_with_mnemonic (
		sessionmanager_translate ("Close", "閉じる")
	);

	gtk_widget_set_sensitive (
		dialog->restore_button,
		FALSE
	);

	gtk_widget_set_sensitive (
		dialog->rename_button,
		FALSE
	);

	gtk_widget_set_sensitive (
		dialog->delete_button,
		FALSE
	);


	gtk_box_pack_start (
		GTK_BOX (button_box),
		dialog->restore_button,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (button_box),
		dialog->rename_button,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (button_box),
		dialog->delete_button,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_end (
		GTK_BOX (button_box),
		close_button,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (content_box),
		dialog->list_box,
		TRUE,
		TRUE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (content_box),
		button_box,
		FALSE,
		FALSE,
		0
	);

	gtk_box_pack_start (
		GTK_BOX (box),
		content_box,
		TRUE,
		TRUE,
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
		dialog->list_box,
		"row-selected",
		G_CALLBACK (sessionmanager_manage_selection_changed),
		dialog
	);

	g_signal_connect (
		dialog->restore_button,
		"clicked",
		G_CALLBACK (sessionmanager_manage_restore_clicked),
		dialog
	);

	g_signal_connect (
		dialog->rename_button,
		"clicked",
		G_CALLBACK (sessionmanager_manage_rename_clicked),
		dialog
	);

	g_signal_connect (
		dialog->delete_button,
		"clicked",
		G_CALLBACK (sessionmanager_manage_delete_clicked),
		dialog
	);

	g_signal_connect (
		dialog->list_box,
		"button-press-event",
		G_CALLBACK (sessionmanager_manage_button_pressed),
		dialog
	);

	g_signal_connect (
		dialog->widget,
		"key-press-event",
		G_CALLBACK (sessionmanager_manage_key_pressed),
		dialog
	);

	g_signal_connect_swapped (
		close_button,
		"clicked",
		G_CALLBACK (gtk_widget_destroy),
		dialog->widget
	);

	if (!sessionmanager_manage_populate_list (
		dialog,
		&error
	))
	{
		g_warning (
			"sessionmanager: Saved sessions could not be loaded: %s",
			error != NULL
				? error->message
				: "unknown error"
		);

		g_clear_error (
			&error
		);
	}

	g_object_set_data_full (
		G_OBJECT (dialog->widget),
		"sessionmanager-manage-dialog",
		dialog,
		(GDestroyNotify)
			sessionmanager_manage_dialog_free
	);

	return dialog;
}

GtkWidget *
sessionmanager_manage_dialog_get_widget (
	SessionmanagerManageDialog *dialog
)
{
	g_return_val_if_fail (
		dialog != NULL,
		NULL
	);

	return dialog->widget;
}

void
sessionmanager_manage_dialog_present (
	SessionmanagerManageDialog *dialog
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

void
sessionmanager_manage_dialog_destroy (
	SessionmanagerManageDialog *dialog
)
{
	g_return_if_fail (
		dialog != NULL
	);

	if (dialog->widget != NULL)
	{
		gtk_widget_destroy (
			dialog->widget
		);
	}
}
