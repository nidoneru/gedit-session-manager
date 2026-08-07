/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "gedit-sessionmanager-app-activatable.h"

#include "sessionmanager-service.h"

#include <glib/gi18n.h>

#include <gedit/gedit-app.h>
#include <gedit/gedit-app-activatable.h>
#include <gedit/gedit-menu-extension.h>
#include "sessionmanager-i18n.h"

struct _GeditSessionmanagerAppActivatablePrivate
{
	GeditApp *app;
	GeditMenuExtension *menu_extension;
};

enum
{
	PROP_0,
	PROP_APP
};

static GeditSessionmanagerAppActivatable *active_instance = NULL;

static void
gedit_app_activatable_iface_init (
	GeditAppActivatableInterface *iface
);

static void
gedit_sessionmanager_app_activatable_activate (
	GeditAppActivatable *activatable
);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (
	GeditSessionmanagerAppActivatable,
	gedit_sessionmanager_app_activatable,
	G_TYPE_OBJECT,
	0,
	G_ADD_PRIVATE_DYNAMIC (
		GeditSessionmanagerAppActivatable
	)
	G_IMPLEMENT_INTERFACE_DYNAMIC (
		GEDIT_TYPE_APP_ACTIVATABLE,
		gedit_app_activatable_iface_init
	)
)

static void
gedit_sessionmanager_app_activatable_get_property (
	GObject    *object,
	guint       prop_id,
	GValue     *value,
	GParamSpec *pspec
)
{
	GeditSessionmanagerAppActivatable *self;

	self = GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (
		object
	);

	switch (prop_id)
	{
		case PROP_APP:
			g_value_set_object (
				value,
				self->priv->app
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
gedit_sessionmanager_app_activatable_set_property (
	GObject      *object,
	guint         prop_id,
	const GValue *value,
	GParamSpec   *pspec
)
{
	GeditSessionmanagerAppActivatable *self;

	self = GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (
		object
	);

	switch (prop_id)
	{
		case PROP_APP:
			g_assert (
				self->priv->app == NULL
			);

			self->priv->app = GEDIT_APP (
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
gedit_sessionmanager_app_activatable_dispose (
	GObject *object
)
{
	GeditSessionmanagerAppActivatable *self;

	self = GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (
		object
	);

	g_clear_object (
		&self->priv->menu_extension
	);

	g_clear_object (
		&self->priv->app
	);

	G_OBJECT_CLASS (
		gedit_sessionmanager_app_activatable_parent_class
	)->dispose (object);
}

static void
gedit_sessionmanager_app_activatable_class_init (
	GeditSessionmanagerAppActivatableClass *klass
)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->get_property =
		gedit_sessionmanager_app_activatable_get_property;

	object_class->set_property =
		gedit_sessionmanager_app_activatable_set_property;

	object_class->dispose =
		gedit_sessionmanager_app_activatable_dispose;

	g_object_class_override_property (
		object_class,
		PROP_APP,
		"app"
	);
}

static void
gedit_sessionmanager_app_activatable_class_finalize (
	GeditSessionmanagerAppActivatableClass *klass
)
{
	(void) klass;
}

static void
gedit_sessionmanager_app_activatable_init (
	GeditSessionmanagerAppActivatable *self
)
{
	self->priv =
		gedit_sessionmanager_app_activatable_get_instance_private (
			self
		);
}

GeditSessionmanagerAppActivatable *
gedit_sessionmanager_app_activatable_get_instance (void)
{
	return active_instance;
}

static gchar *
gedit_sessionmanager_abbreviate_session_name (
	const gchar *name
)
{
	const glong max_chars = 32;
	glong length;
	const gchar *end;

	g_return_val_if_fail (name != NULL, NULL);

	length = g_utf8_strlen (
		name,
		-1
	);

	if (length <= max_chars)
	{
		return g_strdup (
			name
		);
	}

	end = g_utf8_offset_to_pointer (
		name,
		max_chars
	);

	return g_strdup_printf (
		"%.*s…",
		(gint) (end - name),
		name
	);
}

void
gedit_sessionmanager_app_activatable_refresh_menu (
	GeditSessionmanagerAppActivatable *self
)
{
	g_return_if_fail (
		GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (self)
	);

	if (self->priv->menu_extension != NULL)
	{
		gedit_menu_extension_remove_items (
			self->priv->menu_extension
		);
	}

	gedit_sessionmanager_app_activatable_activate (
		GEDIT_APP_ACTIVATABLE (self)
	);
}

static void
gedit_sessionmanager_app_activatable_activate (
	GeditAppActivatable *activatable
)
{
	GeditSessionmanagerAppActivatable *self;
	GMenuItem *item;

	self = GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (
		activatable
	);

	active_instance = self;

	self->priv->menu_extension =
		gedit_app_activatable_extend_menu (
			activatable,
			"tools-section"
		);

	item = g_menu_item_new (
		sessionmanager_translate ("Save Session…", "セッションを保存…"),
		"win.sessionmanager-save-session"
	);

	gedit_menu_extension_append_menu_item (
		self->priv->menu_extension,
		item
	);

	g_object_unref (item);

	gtk_application_set_accels_for_action (
		GTK_APPLICATION (self->priv->app),
		"win.sessionmanager-manage-sessions",
		(const gchar *[]) {
			"<Primary>r",
			NULL
		}
	);

	item = g_menu_item_new (
		sessionmanager_translate ("Manage Sessions…", "セッションを管理…"),
		"win.sessionmanager-manage-sessions"
	);

	gedit_menu_extension_append_menu_item (
		self->priv->menu_extension,
		item
	);

	g_object_unref (item);

	{
		SessionmanagerSessionStore *store;
		GError *error = NULL;
		guint i;

		store = sessionmanager_load_session_store (
			&error
		);

		if (store == NULL)
		{
			g_clear_error (
				&error
			);

			return;
		}

		for (i = 0;
		     i < sessionmanager_session_store_get_n_sessions (
			     store
		     );
		     i++)
		{
			SessionmanagerSession *session;
		gchar *display_name;
			gchar *label;
			GVariant *target;

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

		display_name =
			gedit_sessionmanager_abbreviate_session_name (
				session->name
			);

		label = sessionmanager_format_restore_session (
			display_name
		);

		g_free (
			display_name
		);

			target = g_variant_new_string (
				session->name
			);

			item = g_menu_item_new (
				label,
				NULL
			);

			g_menu_item_set_action_and_target_value (
				item,
				"win.sessionmanager-restore-named",
				target
			);

			gedit_menu_extension_append_menu_item (
				self->priv->menu_extension,
				item
			);

			g_object_unref (
				item
			);

			g_free (
				label
			);
		}

		sessionmanager_session_store_free (
			store
		);
	}
}

static void
gedit_sessionmanager_app_activatable_deactivate (
	GeditAppActivatable *activatable
)
{
	GeditSessionmanagerAppActivatable *self;

	self = GEDIT_SESSIONMANAGER_APP_ACTIVATABLE (
		activatable
	);

	if (active_instance == self)
	{
		active_instance = NULL;
	}

	gtk_application_set_accels_for_action (
		GTK_APPLICATION (self->priv->app),
		"win.sessionmanager-manage-sessions",
		(const gchar *[]) {
			NULL
		}
	);

	g_clear_object (
		&self->priv->menu_extension
	);
}

static void
gedit_app_activatable_iface_init (
	GeditAppActivatableInterface *iface
)
{
	iface->activate =
		gedit_sessionmanager_app_activatable_activate;

	iface->deactivate =
		gedit_sessionmanager_app_activatable_deactivate;
}

void
gedit_sessionmanager_app_activatable_register (
	PeasObjectModule *module
)
{
	gedit_sessionmanager_app_activatable_register_type (
		G_TYPE_MODULE (module)
	);

	peas_object_module_register_extension_type (
		module,
		GEDIT_TYPE_APP_ACTIVATABLE,
		GEDIT_TYPE_SESSIONMANAGER_APP_ACTIVATABLE
	);
}
