/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SessionsaverSaveDialog
	SessionsaverSaveDialog;

typedef void (*SessionsaverSaveRefreshFunc) (
	gpointer user_data
);

SessionsaverSaveDialog *
sessionsaver_save_dialog_new (
	GeditWindow                 *window,
	SessionsaverSaveRefreshFunc  refresh_func,
	gpointer                     user_data
);

GtkWidget *
sessionsaver_save_dialog_get_widget (
	SessionsaverSaveDialog *dialog
);

void
sessionsaver_save_dialog_present (
	SessionsaverSaveDialog *dialog
);

G_END_DECLS
