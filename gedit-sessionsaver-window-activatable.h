/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <libpeas/peas.h>
G_BEGIN_DECLS
#define GEDIT_TYPE_SESSIONSAVER_WINDOW_ACTIVATABLE \
	(gedit_sessionsaver_window_activatable_get_type ())
#define GEDIT_SESSIONSAVER_WINDOW_ACTIVATABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	 GEDIT_TYPE_SESSIONSAVER_WINDOW_ACTIVATABLE, \
	 GeditSessionsaverWindowActivatable))
typedef struct _GeditSessionsaverWindowActivatable
	GeditSessionsaverWindowActivatable;
typedef struct _GeditSessionsaverWindowActivatableClass
	GeditSessionsaverWindowActivatableClass;
typedef struct _GeditSessionsaverWindowActivatablePrivate
	GeditSessionsaverWindowActivatablePrivate;
struct _GeditSessionsaverWindowActivatable
{
	GObject parent;
	GeditSessionsaverWindowActivatablePrivate *priv;
};
struct _GeditSessionsaverWindowActivatableClass
{
	GObjectClass parent_class;
};
GType
gedit_sessionsaver_window_activatable_get_type (void);
void
gedit_sessionsaver_window_activatable_register (PeasObjectModule *module);
G_END_DECLS
