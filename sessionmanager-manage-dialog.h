/* SPDX-License-Identifier: MIT */

#pragma once

#include <gedit/gedit-window.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SessionmanagerManageDialog
	SessionmanagerManageDialog;

typedef void (*SessionmanagerManageRestoreFunc) (
	GeditWindow *window,
	const gchar *session_name,
	gpointer     user_data
);

typedef void (*SessionmanagerManageRefreshFunc) (
	gpointer user_data
);

SessionmanagerManageDialog *
sessionmanager_manage_dialog_new (
	GeditWindow                    *window,
	SessionmanagerManageRestoreFunc   restore_func,
	SessionmanagerManageRefreshFunc   refresh_func,
	gpointer                        user_data
);

GtkWidget *
sessionmanager_manage_dialog_get_widget (
	SessionmanagerManageDialog *dialog
);

void
sessionmanager_manage_dialog_present (
	SessionmanagerManageDialog *dialog
);

void
sessionmanager_manage_dialog_destroy (
	SessionmanagerManageDialog *dialog
);

G_END_DECLS
