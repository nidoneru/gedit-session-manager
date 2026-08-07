/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>

#include "sessionmanager-session.h"

G_BEGIN_DECLS

SessionmanagerSession *
sessionmanager_capture_window (
	GeditWindow *window,
	const gchar *name
);

gboolean
sessionmanager_restore_window (
	GeditWindow               *window,
	const SessionmanagerSession *session
);

G_END_DECLS
