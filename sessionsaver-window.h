/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <gedit/gedit-window.h>

#include "sessionsaver-session.h"

G_BEGIN_DECLS

SessionsaverSession *
sessionsaver_capture_window (
	GeditWindow *window,
	const gchar *name
);

gboolean
sessionsaver_restore_window (
	GeditWindow               *window,
	const SessionsaverSession *session
);

G_END_DECLS
