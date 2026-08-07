/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SessionsaverManageDialog
	SessionsaverManageDialog;

typedef void (*SessionsaverManageRestoreFunc) (
	GeditWindow *window,
	const gchar *session_name,
	gpointer     user_data
);

typedef void (*SessionsaverManageRefreshFunc) (
	gpointer user_data
);

SessionsaverManageDialog *
sessionsaver_manage_dialog_new (
	GeditWindow                    *window,
	SessionsaverManageRestoreFunc   restore_func,
	SessionsaverManageRefreshFunc   refresh_func,
	gpointer                        user_data
);

GtkWidget *
sessionsaver_manage_dialog_get_widget (
	SessionsaverManageDialog *dialog
);

void
sessionsaver_manage_dialog_present (
	SessionsaverManageDialog *dialog
);

void
sessionsaver_manage_dialog_destroy (
	SessionsaverManageDialog *dialog
);

G_END_DECLS
