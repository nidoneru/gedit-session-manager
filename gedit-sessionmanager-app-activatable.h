/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_SESSIONMANAGER_APP_ACTIVATABLE \
	(gedit_sessionmanager_app_activatable_get_type ())

#define GEDIT_SESSIONMANAGER_APP_ACTIVATABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ( \
		(obj), \
		GEDIT_TYPE_SESSIONMANAGER_APP_ACTIVATABLE, \
		GeditSessionmanagerAppActivatable \
	))

typedef struct _GeditSessionmanagerAppActivatable
	GeditSessionmanagerAppActivatable;

typedef struct _GeditSessionmanagerAppActivatableClass
	GeditSessionmanagerAppActivatableClass;

typedef struct _GeditSessionmanagerAppActivatablePrivate
	GeditSessionmanagerAppActivatablePrivate;

struct _GeditSessionmanagerAppActivatable
{
	GObject parent;

	GeditSessionmanagerAppActivatablePrivate *priv;
};

struct _GeditSessionmanagerAppActivatableClass
{
	GObjectClass parent_class;
};

GType
gedit_sessionmanager_app_activatable_get_type (void);

void
gedit_sessionmanager_app_activatable_register (
	PeasObjectModule *module
);

void
gedit_sessionmanager_app_activatable_refresh_menu (
	GeditSessionmanagerAppActivatable *self
);

GeditSessionmanagerAppActivatable *
gedit_sessionmanager_app_activatable_get_instance (void);

G_END_DECLS
