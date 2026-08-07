/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_SESSIONSAVER_APP_ACTIVATABLE \
	(gedit_sessionsaver_app_activatable_get_type ())

#define GEDIT_SESSIONSAVER_APP_ACTIVATABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ( \
		(obj), \
		GEDIT_TYPE_SESSIONSAVER_APP_ACTIVATABLE, \
		GeditSessionsaverAppActivatable \
	))

typedef struct _GeditSessionsaverAppActivatable
	GeditSessionsaverAppActivatable;

typedef struct _GeditSessionsaverAppActivatableClass
	GeditSessionsaverAppActivatableClass;

typedef struct _GeditSessionsaverAppActivatablePrivate
	GeditSessionsaverAppActivatablePrivate;

struct _GeditSessionsaverAppActivatable
{
	GObject parent;

	GeditSessionsaverAppActivatablePrivate *priv;
};

struct _GeditSessionsaverAppActivatableClass
{
	GObjectClass parent_class;
};

GType
gedit_sessionsaver_app_activatable_get_type (void);

void
gedit_sessionsaver_app_activatable_register (
	PeasObjectModule *module
);

void
gedit_sessionsaver_app_activatable_refresh_menu (
	GeditSessionsaverAppActivatable *self
);

GeditSessionsaverAppActivatable *
gedit_sessionsaver_app_activatable_get_instance (void);

G_END_DECLS
