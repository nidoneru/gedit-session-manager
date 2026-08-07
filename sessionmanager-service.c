/* SPDX-License-Identifier: MIT */

#include "sessionmanager-service.h"

#include "sessionmanager-storage.h"
#include "sessionmanager-window.h"

#include <gedit/gedit-commands.h>
#include <gedit/gedit-document.h>
#include "sessionmanager-i18n.h"

typedef struct
{
	GeditWindow *window;
	gchar *session_name;

	GList *documents;
	GList *current;
} SessionmanagerReplaceData;

static void
sessionmanager_replace_data_free (
	SessionmanagerReplaceData *data
)
{
	if (data == NULL)
	{
		return;
	}

	g_clear_object (
		&data->window
	);

	g_clear_pointer (
		&data->session_name,
		g_free
	);

	g_list_free_full (
		data->documents,
		g_object_unref
	);

	g_free (data);
}

gboolean
sessionmanager_save_session (
	SessionmanagerSession *session,
	GError             **error
)
{
	SessionmanagerSessionStore *store;

	g_return_val_if_fail (session != NULL, FALSE);
	g_return_val_if_fail (session->name != NULL, FALSE);
	g_return_val_if_fail (session->name[0] != '\0', FALSE);

	store = sessionmanager_storage_load_or_new (
		error
	);

	if (store == NULL)
	{
		sessionmanager_session_free (session);
		return FALSE;
	}

	/*
	 * sessionの所有権はSessionStoreへ移る。
	 * 同名セッションがある場合は置き換える。
	 */
	sessionmanager_storage_replace_session (
		store,
		session
	);

	if (!sessionmanager_storage_save (
		store,
		error
	))
	{
		sessionmanager_session_store_free (store);
		return FALSE;
	}

	sessionmanager_session_store_free (store);

	return TRUE;
}

gboolean
sessionmanager_rename_session (
	const gchar *old_name,
	const gchar *new_name,
	GError     **error
)
{
	SessionmanagerSessionStore *store;
	gboolean renamed;

	g_return_val_if_fail (old_name != NULL, FALSE);
	g_return_val_if_fail (old_name[0] != '\0', FALSE);
	g_return_val_if_fail (new_name != NULL, FALSE);
	g_return_val_if_fail (new_name[0] != '\0', FALSE);

	if (g_ascii_strcasecmp (
		old_name,
		SESSIONMANAGER_PREVIOUS_SESSION_NAME
	) == 0)
	{
		g_set_error (
			error,
			G_IO_ERROR,
			G_IO_ERROR_PERMISSION_DENIED,
			"Previous Session の名前は変更できません。"
		);

		return FALSE;
	}

	if (g_ascii_strcasecmp (
		new_name,
		SESSIONMANAGER_PREVIOUS_SESSION_NAME
	) == 0)
	{
		g_set_error (
			error,
			G_IO_ERROR,
			G_IO_ERROR_EXISTS,
			"Previous Session は予約済みの名前です。"
		);

		return FALSE;
	}

	store = sessionmanager_storage_load (
		error
	);

	if (store == NULL)
	{

		return FALSE;
	}

	renamed = sessionmanager_session_store_rename (
		store,
		old_name,
		new_name
	);

	if (!renamed)
	{
		{
			gchar *message;

			message = sessionmanager_format_duplicate_session_name (
				new_name
			);

			g_set_error (
				error,
				G_IO_ERROR,
				G_IO_ERROR_EXISTS,
				"%s",
				message
			);

			g_free (
				message
			);
		}

		sessionmanager_session_store_free (store);
		return FALSE;
	}

	if (!sessionmanager_storage_save (
		store,
		error
	))
	{
		sessionmanager_session_store_free (store);
		return FALSE;
	}

	sessionmanager_session_store_free (store);

	return TRUE;
}

gboolean
sessionmanager_delete_session (
	const gchar *name,
	GError     **error
)
{
	SessionmanagerSessionStore *store;

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (name[0] != '\0', FALSE);

	if (g_ascii_strcasecmp (
		name,
		SESSIONMANAGER_PREVIOUS_SESSION_NAME
	) == 0)
	{
		g_set_error (
			error,
			G_IO_ERROR,
			G_IO_ERROR_PERMISSION_DENIED,
			"Previous Session は削除できません。"
		);

		return FALSE;
	}

	store = sessionmanager_storage_load (
		error
	);

	if (store == NULL)
	{
		return FALSE;
	}

	if (!sessionmanager_session_store_remove (
		store,
		name
	))
	{
		g_set_error (
			error,
			G_IO_ERROR,
			G_IO_ERROR_NOT_FOUND,
			"セッションが見つかりません: %s",
			name
		);

		sessionmanager_session_store_free (store);
		return FALSE;
	}

	if (!sessionmanager_storage_save (
		store,
		error
	))
	{
		sessionmanager_session_store_free (store);
		return FALSE;
	}

	sessionmanager_session_store_free (store);

	return TRUE;
}

SessionmanagerSessionStore *
sessionmanager_load_session_store (
	GError **error
)
{
	return sessionmanager_storage_load (error);
}

gboolean
sessionmanager_restore_session (
	GeditWindow *window,
	const gchar *name,
	GError     **error
)
{
	SessionmanagerSessionStore *store;
	SessionmanagerSession *session;
	gboolean restored;

	g_return_val_if_fail (GEDIT_IS_WINDOW (window), FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (name[0] != '\0', FALSE);

	store = sessionmanager_storage_load (
		error
	);

	if (store == NULL)
	{
		return FALSE;
	}

	session = sessionmanager_session_store_find (
		store,
		name
	);

	if (session == NULL)
	{
		g_set_error (
			error,
			G_IO_ERROR,
			G_IO_ERROR_NOT_FOUND,
			"セッションが見つかりません: %s",
			name
		);

		sessionmanager_session_store_free (store);
		return FALSE;
	}

	restored = sessionmanager_restore_window (
		window,
		session
	);

	sessionmanager_session_store_free (store);

	return restored;
}

static void
sessionmanager_replace_continue (
	GTask *task
);

static void
sessionmanager_replace_document_saved (
	GeditDocument *document,
	GAsyncResult  *result,
	gpointer       user_data
)
{
	GTask *task;
	SessionmanagerReplaceData *data;

	task = G_TASK (user_data);
	data = g_task_get_task_data (task);

	if (!gedit_commands_save_document_finish (
		document,
		result
	))
	{
		g_task_return_new_error (
			task,
			G_IO_ERROR,
			G_IO_ERROR_CANCELLED,
			"文書の保存がキャンセルされたか、保存に失敗しました。"
		);

		g_object_unref (task);
		return;
	}

	data->current = g_list_next (
		data->current
	);

	sessionmanager_replace_continue (
		task
	);
}

static void
sessionmanager_replace_continue (
	GTask *task
)
{
	SessionmanagerReplaceData *data;

	data = g_task_get_task_data (task);

	while (data->current != NULL)
	{
		GeditDocument *document;

		document = GEDIT_DOCUMENT (
			data->current->data
		);

		/*
		 * 変更されていない文書は保存不要。
		 *
		 * 空で変更されていない無題文書もここでスキップされ、
		 * 後ほどそのまま閉じられる。
		 */
		if (!gtk_text_buffer_get_modified (
			GTK_TEXT_BUFFER (document)
		))
		{
			data->current = g_list_next (
				data->current
			);

			continue;
		}

		gedit_commands_save_document_async (
			document,
			data->window,
			g_task_get_cancellable (task),
			(GAsyncReadyCallback)
				sessionmanager_replace_document_saved,
			g_object_ref (task)
		);

		/*
		 * 非同期コールバックへ参照を引き渡したので、
		 * 現在の処理が保持している参照を解放する。
		 */
		g_object_unref (
			task
		);

		return;
	}

	/*
	 * すべての変更済み文書を保存できたので、
	 * 現在のタブを閉じて指定セッションへ置き換える。
	 */
	gedit_window_close_all_tabs (
		data->window
	);

	{
		GError *error = NULL;

		if (!sessionmanager_restore_session (
			data->window,
			data->session_name,
			&error
		))
		{
			g_task_return_error (
				task,
				error
			);

			g_object_unref (task);
			return;
		}
	}

	g_task_return_boolean (
		task,
		TRUE
	);

	g_object_unref (task);
}

void
sessionmanager_replace_with_session_async (
	GeditWindow         *window,
	const gchar         *name,
	GCancellable        *cancellable,
	GAsyncReadyCallback  callback,
	gpointer             user_data
)
{
	GTask *task;
	SessionmanagerReplaceData *data;
	GList *documents;
	GList *item;

	g_return_if_fail (GEDIT_IS_WINDOW (window));
	g_return_if_fail (name != NULL);
	g_return_if_fail (name[0] != '\0');
	g_return_if_fail (
		cancellable == NULL ||
		G_IS_CANCELLABLE (cancellable)
	);

	task = g_task_new (
		window,
		cancellable,
		callback,
		user_data
	);

	data = g_new0 (
		SessionmanagerReplaceData,
		1
	);

	data->window = g_object_ref (
		window
	);

	data->session_name = g_strdup (
		name
	);

	documents = gedit_window_get_documents (
		window
	);

	for (item = documents; item != NULL; item = item->next)
	{
		data->documents = g_list_append (
			data->documents,
			g_object_ref (item->data)
		);
	}

	g_list_free (
		documents
	);

	data->current = data->documents;

	g_task_set_task_data (
		task,
		data,
		(GDestroyNotify)
			sessionmanager_replace_data_free
	);

	sessionmanager_replace_continue (
		task
	);
}

gboolean
sessionmanager_replace_with_session_finish (
	GeditWindow  *window,
	GAsyncResult *result,
	GError      **error
)
{
	g_return_val_if_fail (
		g_task_is_valid (
			result,
			window
		),
		FALSE
	);

	return g_task_propagate_boolean (
		G_TASK (result),
		error
	);
}
