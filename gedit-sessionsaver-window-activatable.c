/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "gedit-sessionsaver-window-activatable.h"
#include "gedit-sessionsaver-app-activatable.h"

#include "sessionsaver-manage-dialog.h"
#include "sessionsaver-save-dialog.h"
#include "sessionsaver-service.h"
#include "sessionsaver-window.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <gedit/gedit-document.h>
#include <gedit/gedit-tab.h>
#include <gedit/gedit-window.h>
#include <gedit/gedit-window-activatable.h>
#include <gtksourceview/gtksource.h>
#include "sessionsaver-i18n.h"

struct _GeditSessionsaverWindowActivatablePrivate
{
	GeditWindow *window;

	gulong tab_added_handler_id;
	gulong tab_removed_handler_id;

	guint save_idle_id;
	guint restore_idle_id;

	GSimpleAction *save_session_action;
	GSimpleAction *restore_named_session_action;
	GSimpleAction *manage_sessions_action;

	GtkWidget *save_session_dialog;
	GtkWidget *manage_sessions_dialog;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static void
gedit_window_activatable_iface_init (
	GeditWindowActivatableInterface *iface
);

G_MODULE_EXPORT void
peas_register_types (
	PeasObjectModule *module
);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (
	GeditSessionsaverWindowActivatable,
	gedit_sessionsaver_window_activatable,
	G_TYPE_OBJECT,
	0,
	G_ADD_PRIVATE_DYNAMIC (
		GeditSessionsaverWindowActivatable
	)
	G_IMPLEMENT_INTERFACE_DYNAMIC (
		GEDIT_TYPE_WINDOW_ACTIVATABLE,
		gedit_window_activatable_iface_init
	)
)

static void
sessionsaver_restore_session_finished (
	GeditWindow  *window,
	GAsyncResult *result,
	gpointer      user_data
);

static void
sessionsaver_refresh_app_menu (void);

static void
sessionsaver_manage_restore_requested (
	GeditWindow *window,
	const gchar *session_name,
	gpointer     user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	sessionsaver_replace_with_session_async (
		window,
		session_name,
		NULL,
		(GAsyncReadyCallback)
			sessionsaver_restore_session_finished,
		self
	);
}

static void
sessionsaver_manage_refresh_requested (
	gpointer user_data
)
{
	(void) user_data;

	sessionsaver_refresh_app_menu ();
}

static void
sessionsaver_refresh_app_menu (void)
{
	GeditSessionsaverAppActivatable *app_activatable;

	app_activatable =
		gedit_sessionsaver_app_activatable_get_instance ();

	if (app_activatable != NULL)
	{
		gedit_sessionsaver_app_activatable_refresh_menu (
			app_activatable
		);
	}
}

/* ---- Previous Session の復元 ---- */

static void
sessionsaver_restore_previous_session (
	GeditWindow *window
)
{
	GError *error = NULL;

	if (!sessionsaver_restore_session (
		window,
		SESSIONSAVER_PREVIOUS_SESSION_NAME,
		&error
	))
	{
		if (error != NULL)
		{
			g_warning (
				"sessionsaver: Previous Sessionの復元に失敗しました: %s",
				error->message
			);

			g_clear_error (&error);
		}
	}
}

/* ---- Previous Session の遅延復元 ---- */

static gboolean
sessionsaver_restore_previous_idle (
	gpointer user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	self->priv->restore_idle_id = 0;

	if (self->priv->window != NULL)
	{
		sessionsaver_restore_previous_session (
			self->priv->window
		);
	}

	return G_SOURCE_REMOVE;
}

/* ---- Previous Session の遅延保存 ---- */

static gboolean
sessionsaver_save_previous_idle (
	gpointer user_data
)
{
	GeditSessionsaverWindowActivatable *self;
	SessionsaverSession *session;
	GError *error = NULL;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	self->priv->save_idle_id = 0;

	if (self->priv->window == NULL)
	{
		return G_SOURCE_REMOVE;
	}

	session = sessionsaver_capture_window (
		self->priv->window,
		SESSIONSAVER_PREVIOUS_SESSION_NAME
	);

	if (session == NULL)
	{
		return G_SOURCE_REMOVE;
	}

	/*
	 * sessionsaver_save_session() は、成功・失敗にかかわらず
	 * sessionの所有権を引き取る。
	 */
	if (!sessionsaver_save_session (
		session,
		&error
	))
	{
		g_warning (
			"sessionsaver: Previous Sessionの保存に失敗しました: %s",
			error != NULL ? error->message : "unknown error"
		);

		g_clear_error (&error);
	}

	return G_SOURCE_REMOVE;
}

static void
sessionsaver_schedule_save (
	GeditSessionsaverWindowActivatable *self
)
{
	if (self->priv->save_idle_id != 0)
	{
		return;
	}

	self->priv->save_idle_id = g_idle_add_full (
		G_PRIORITY_DEFAULT_IDLE,
		sessionsaver_save_previous_idle,
		g_object_ref (self),
		g_object_unref
	);
}

/* ---- ドキュメントの保存先変更を監視 ---- */

static void
on_document_location_changed (
	GtkSourceFile *source_file,
	GParamSpec    *pspec,
	gpointer       user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	(void) source_file;
	(void) pspec;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	sessionsaver_schedule_save (self);
}

static void
sessionsaver_watch_document (
	GeditSessionsaverWindowActivatable *self,
	GeditDocument                      *document
)
{
	GtkSourceFile *source_file;

	source_file = gedit_document_get_file (
		document
	);

	/*
	 * 同じGtkSourceFileへの重複接続を避ける。
	 */
	g_signal_handlers_disconnect_by_func (
		source_file,
		G_CALLBACK (on_document_location_changed),
		self
	);

	g_signal_connect_object (
		source_file,
		"notify::location",
		G_CALLBACK (on_document_location_changed),
		self,
		0
	);
}

/* ---- タブ構成の変更を監視 ---- */

static void
on_tab_added (
	GeditWindow *window,
	GeditTab    *tab,
	gpointer     user_data
)
{
	GeditSessionsaverWindowActivatable *self;
	GeditDocument *document;

	(void) window;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	document = gedit_tab_get_document (tab);

	sessionsaver_watch_document (
		self,
		document
	);

	sessionsaver_schedule_save (self);
}

static void
on_tab_removed (
	GeditWindow *window,
	GeditTab    *tab,
	gpointer     user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	(void) window;
	(void) tab;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	/*
	 * シグナル中には保存せず、タブ構成の変更が完了したあとに保存する。
	 */
	sessionsaver_schedule_save (self);
}

/* ---- 名前付きセッションの保存アクション ---- */

static void
sessionsaver_message_dialog_response (
	GtkDialog *dialog,
	gint       response_id,
	gpointer   user_data
)
{
	(void) response_id;
	(void) user_data;

	gtk_widget_destroy (
		GTK_WIDGET (dialog)
	);
}

static void
sessionsaver_show_message (
	GeditWindow    *window,
	GtkMessageType  type,
	const gchar    *primary_text,
	const gchar    *secondary_text
)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (
		GTK_WINDOW (window),
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
			GTK_MESSAGE_DIALOG (dialog),
			"%s",
			secondary_text
		);
	}

	g_signal_connect (
		dialog,
		"response",
		G_CALLBACK (sessionsaver_message_dialog_response),
		NULL
	);

	gtk_widget_show (dialog);
}

static void
on_restore_named_session_action (
	GSimpleAction *action,
	GVariant      *parameter,
	gpointer       user_data
)
{
	GeditSessionsaverWindowActivatable *self;
	const gchar *session_name;

	(void) action;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	if (parameter == NULL)
	{
		return;
	}

	session_name = g_variant_get_string (
		parameter,
		NULL
	);

	sessionsaver_replace_with_session_async (
		self->priv->window,
		session_name,
		NULL,
		(GAsyncReadyCallback)
			sessionsaver_restore_session_finished,
		self
	);
}

static void
sessionsaver_restore_session_finished (
	GeditWindow *window,
	GAsyncResult *result,
	gpointer      user_data
)
{
	GeditSessionsaverWindowActivatable *self;
	GError *error = NULL;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	if (!sessionsaver_replace_with_session_finish (
		window,
		result,
		&error
	))
	{
		/*
		 * 保存ダイアログのキャンセルもFALSEとして返る。
		 * エラー内容はgedit側でも表示されるため、ここでは
		 * セッション切り替えが完了しなかったことだけ通知する。
		 */
		sessionsaver_show_message (
			self->priv->window,
			GTK_MESSAGE_WARNING,
			sessionsaver_translate ("The session could not be restored.", "セッションを復元できません。"),
			error != NULL
				? error->message
				: NULL
		);

		g_clear_error (&error);
	}
}

static void
sessionsaver_manage_dialog_destroyed (
	GtkWidget *dialog,
	gpointer   user_data
);

static void
sessionsaver_manage_dialog_show (
	GeditSessionsaverWindowActivatable *self
)
{
	SessionsaverManageDialog *manage_dialog;
	GtkWidget *widget;

	if (self->priv->manage_sessions_dialog != NULL)
	{
		gtk_window_present (
			GTK_WINDOW (self->priv->manage_sessions_dialog)
		);

		return;
	}

	manage_dialog = sessionsaver_manage_dialog_new (
		self->priv->window,
		sessionsaver_manage_restore_requested,
		sessionsaver_manage_refresh_requested,
		self
	);

	if (manage_dialog == NULL)
	{
		return;
	}

	widget = sessionsaver_manage_dialog_get_widget (
		manage_dialog
	);

	self->priv->manage_sessions_dialog = widget;

	g_signal_connect (
		widget,
		"destroy",
		G_CALLBACK (sessionsaver_manage_dialog_destroyed),
		self
	);

	gtk_widget_show_all (
		widget
	);
}

static void
on_manage_sessions_action (
	GSimpleAction *action,
	GVariant      *parameter,
	gpointer       user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	(void) action;
	(void) parameter;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	sessionsaver_manage_dialog_show (
		self
	);
}

static void
sessionsaver_manage_dialog_destroyed (
	GtkWidget *dialog,
	gpointer   user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	if (self->priv->manage_sessions_dialog == dialog)
	{
		self->priv->manage_sessions_dialog = NULL;
	}
}

static void
sessionsaver_save_dialog_destroyed (
	GtkWidget *dialog,
	gpointer   user_data
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	if (self->priv->save_session_dialog == dialog)
	{
		self->priv->save_session_dialog = NULL;
	}
}

static void
on_save_session_action (
	GSimpleAction *action,
	GVariant      *parameter,
	gpointer       user_data
)
{
	GeditSessionsaverWindowActivatable *self;
	SessionsaverSaveDialog *save_dialog;
	GtkWidget *widget;

	(void) action;
	(void) parameter;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		user_data
	);

	if (self->priv->save_session_dialog != NULL)
	{
		gtk_window_present (
			GTK_WINDOW (self->priv->save_session_dialog)
		);

		return;
	}

	save_dialog = sessionsaver_save_dialog_new (
		self->priv->window,
		sessionsaver_manage_refresh_requested,
		self
	);

	if (save_dialog == NULL)
	{
		return;
	}

	widget = sessionsaver_save_dialog_get_widget (
		save_dialog
	);

	self->priv->save_session_dialog = widget;

	g_signal_connect (
		widget,
		"destroy",
		G_CALLBACK (sessionsaver_save_dialog_destroyed),
		self
	);

	gtk_widget_show_all (
		widget
	);

}

/* ---- GObject ---- */

static void
gedit_sessionsaver_window_activatable_get_property (
	GObject    *object,
	guint       prop_id,
	GValue     *value,
	GParamSpec *pspec
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		object
	);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (
				value,
				self->priv->window
			);

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (
				object,
				prop_id,
				pspec
			);

			break;
	}
}

static void
gedit_sessionsaver_window_activatable_set_property (
	GObject      *object,
	guint         prop_id,
	const GValue *value,
	GParamSpec   *pspec
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		object
	);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_assert (
				self->priv->window == NULL
			);

			self->priv->window = GEDIT_WINDOW (
				g_value_dup_object (value)
			);

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (
				object,
				prop_id,
				pspec
			);

			break;
	}
}

static void
gedit_sessionsaver_window_activatable_dispose (
	GObject *object
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		object
	);

	if (self->priv->restore_idle_id != 0)
	{
		g_source_remove (
			self->priv->restore_idle_id
		);

		self->priv->restore_idle_id = 0;
	}

	if (self->priv->save_idle_id != 0)
	{
		g_source_remove (
			self->priv->save_idle_id
		);

		self->priv->save_idle_id = 0;
	}

	if (self->priv->save_session_dialog != NULL)
	{
		gtk_widget_destroy (
			self->priv->save_session_dialog
		);

		self->priv->save_session_dialog = NULL;
	}


	if (self->priv->manage_sessions_dialog != NULL)
	{
		gtk_widget_destroy (
			self->priv->manage_sessions_dialog
		);

		self->priv->manage_sessions_dialog = NULL;
	}

	g_clear_object (
		&self->priv->save_session_action
	);

	g_clear_object (
		&self->priv->restore_named_session_action
	);

	g_clear_object (
		&self->priv->manage_sessions_action
	);

	g_clear_object (
		&self->priv->window
	);

	G_OBJECT_CLASS (
		gedit_sessionsaver_window_activatable_parent_class
	)->dispose (object);
}

static void
gedit_sessionsaver_window_activatable_class_init (
	GeditSessionsaverWindowActivatableClass *klass
)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->get_property =
		gedit_sessionsaver_window_activatable_get_property;

	object_class->set_property =
		gedit_sessionsaver_window_activatable_set_property;

	object_class->dispose =
		gedit_sessionsaver_window_activatable_dispose;

	g_object_class_override_property (
		object_class,
		PROP_WINDOW,
		"window"
	);
}

static void
gedit_sessionsaver_window_activatable_class_finalize (
	GeditSessionsaverWindowActivatableClass *klass
)
{
	(void) klass;
}

static void
gedit_sessionsaver_window_activatable_init (
	GeditSessionsaverWindowActivatable *self
)
{
	self->priv =
		gedit_sessionsaver_window_activatable_get_instance_private (
			self
		);

	self->priv->tab_added_handler_id = 0;
	self->priv->tab_removed_handler_id = 0;
	self->priv->save_idle_id = 0;
	self->priv->restore_idle_id = 0;
	self->priv->save_session_action = NULL;
	self->priv->restore_named_session_action = NULL;
	self->priv->manage_sessions_action = NULL;
	self->priv->save_session_dialog = NULL;
	self->priv->manage_sessions_dialog = NULL;
}

/* ---- GeditWindowActivatable ---- */

static void
gedit_sessionsaver_window_activatable_activate (
	GeditWindowActivatable *activatable
)
{
	GeditSessionsaverWindowActivatable *self;
	GList *documents;
	GList *item;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		activatable
	);

	self->priv->save_session_action = g_simple_action_new (
		"sessionsaver-save-session",
		NULL
	);

	g_signal_connect (
		self->priv->save_session_action,
		"activate",
		G_CALLBACK (on_save_session_action),
		self
	);

	g_action_map_add_action (
		G_ACTION_MAP (self->priv->window),
		G_ACTION (self->priv->save_session_action)
	);


	self->priv->restore_named_session_action =
		g_simple_action_new (
			"sessionsaver-restore-named",
			G_VARIANT_TYPE_STRING
		);

	g_signal_connect (
		self->priv->restore_named_session_action,
		"activate",
		G_CALLBACK (on_restore_named_session_action),
		self
	);

	g_action_map_add_action (
		G_ACTION_MAP (self->priv->window),
		G_ACTION (
			self->priv->restore_named_session_action
		)
	);

	self->priv->manage_sessions_action = g_simple_action_new (
		"sessionsaver-manage-sessions",
		NULL
	);

	g_signal_connect (
		self->priv->manage_sessions_action,
		"activate",
		G_CALLBACK (on_manage_sessions_action),
		self
	);

	g_action_map_add_action (
		G_ACTION_MAP (self->priv->window),
		G_ACTION (self->priv->manage_sessions_action)
	);

	self->priv->tab_added_handler_id = g_signal_connect (
		self->priv->window,
		"tab-added",
		G_CALLBACK (on_tab_added),
		self
	);

	self->priv->tab_removed_handler_id = g_signal_connect (
		self->priv->window,
		"tab-removed",
		G_CALLBACK (on_tab_removed),
		self
	);

	/*
	 * プラグインを有効にした時点ですでに開いている
	 * ドキュメントについても保存先の変更を監視する。
	 */
	documents = gedit_window_get_documents (
		self->priv->window
	);

	for (item = documents; item != NULL; item = item->next)
	{
		sessionsaver_watch_document (
			self,
			GEDIT_DOCUMENT (item->data)
		);
	}

	g_list_free (documents);

	self->priv->restore_idle_id = g_idle_add_full (
		G_PRIORITY_DEFAULT_IDLE,
		sessionsaver_restore_previous_idle,
		g_object_ref (self),
		g_object_unref
	);
}

static void
gedit_sessionsaver_window_activatable_deactivate (
	GeditWindowActivatable *activatable
)
{
	GeditSessionsaverWindowActivatable *self;

	self = GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE (
		activatable
	);

	g_action_map_remove_action (
		G_ACTION_MAP (self->priv->window),
		"sessionsaver-save-session"
	);

	g_action_map_remove_action (
		G_ACTION_MAP (self->priv->window),
		"sessionsaver-restore-named"
	);

	g_action_map_remove_action (
		G_ACTION_MAP (self->priv->window),
		"sessionsaver-manage-sessions"
	);

	g_clear_object (
		&self->priv->save_session_action
	);

	g_clear_object (
		&self->priv->restore_named_session_action
	);

	g_clear_object (
		&self->priv->manage_sessions_action
	);

	if (self->priv->restore_idle_id != 0)
	{
		g_source_remove (
			self->priv->restore_idle_id
		);

		self->priv->restore_idle_id = 0;
	}

	if (self->priv->save_idle_id != 0)
	{
		g_source_remove (
			self->priv->save_idle_id
		);

		self->priv->save_idle_id = 0;
	}

	if (self->priv->tab_added_handler_id != 0)
	{
		g_signal_handler_disconnect (
			self->priv->window,
			self->priv->tab_added_handler_id
		);

		self->priv->tab_added_handler_id = 0;
	}

	if (self->priv->tab_removed_handler_id != 0)
	{
		g_signal_handler_disconnect (
			self->priv->window,
			self->priv->tab_removed_handler_id
		);

		self->priv->tab_removed_handler_id = 0;
	}
}

static void
gedit_window_activatable_iface_init (
	GeditWindowActivatableInterface *iface
)
{
	iface->activate =
		gedit_sessionsaver_window_activatable_activate;

	iface->deactivate =
		gedit_sessionsaver_window_activatable_deactivate;
}

/* ---- libpeas ---- */

void
gedit_sessionsaver_window_activatable_register (
	PeasObjectModule *module
)
{
	gedit_sessionsaver_window_activatable_register_type (
		G_TYPE_MODULE (module)
	);

	peas_object_module_register_extension_type (
		module,
		GEDIT_TYPE_WINDOW_ACTIVATABLE,
		GEDIT_TYPE_SESSIONSAVER_WINDOW_ACTIVATABLE
	);
}

G_MODULE_EXPORT void
peas_register_types (
	PeasObjectModule *module
)
{
	gedit_sessionsaver_app_activatable_register (
		module
	);

	gedit_sessionsaver_window_activatable_register (
		module
	);
}
