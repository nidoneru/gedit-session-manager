/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SessionmanagerSaveDialog
	SessionmanagerSaveDialog;

typedef void (*SessionmanagerSaveRefreshFunc) (
	gpointer user_data
);

SessionmanagerSaveDialog *
sessionmanager_save_dialog_new (
	GeditWindow                 *window,
	SessionmanagerSaveRefreshFunc  refresh_func,
	gpointer                     user_data
);

GtkWidget *
sessionmanager_save_dialog_get_widget (
	SessionmanagerSaveDialog *dialog
);

void
sessionmanager_save_dialog_present (
	SessionmanagerSaveDialog *dialog
);

G_END_DECLS
