/* SPDX-License-Identifier: MIT */
#pragma once
#include <libpeas/peas.h>
G_BEGIN_DECLS
#define GEDIT_TYPE_SESSIONMANAGER_WINDOW_ACTIVATABLE \
	(gedit_sessionmanager_window_activatable_get_type ())
#define GEDIT_SESSIONMANAGER_WINDOW_ACTIVATABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	 GEDIT_TYPE_SESSIONMANAGER_WINDOW_ACTIVATABLE, \
	 GeditSessionmanagerWindowActivatable))
typedef struct _GeditSessionmanagerWindowActivatable
	GeditSessionmanagerWindowActivatable;
typedef struct _GeditSessionmanagerWindowActivatableClass
	GeditSessionmanagerWindowActivatableClass;
typedef struct _GeditSessionmanagerWindowActivatablePrivate
	GeditSessionmanagerWindowActivatablePrivate;
struct _GeditSessionmanagerWindowActivatable
{
	GObject parent;
	GeditSessionmanagerWindowActivatablePrivate *priv;
};
struct _GeditSessionmanagerWindowActivatableClass
{
	GObjectClass parent_class;
};
GType
gedit_sessionmanager_window_activatable_get_type (void);
void
gedit_sessionmanager_window_activatable_register (PeasObjectModule *module);
G_END_DECLS
