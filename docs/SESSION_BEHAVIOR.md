# Session Saver Behavior

## Previous Session

`Previous Session` is a reserved session.

It represents the most recently captured set of open saved documents.

The session is restored automatically after the gedit window has been
activated.

Users cannot create, rename, or delete a named session using this reserved
name.

## Updating Previous Session

`Previous Session` is updated after changes that may affect the set of saved
documents represented by the current window.

The current implementation schedules an idle save after:

- a tab is added;
- a tab is removed;
- a document obtains or changes its saved location.

The save is deferred with an idle callback so that the surrounding tab or
document operation can complete first.

If another save is already scheduled, an additional idle source is not
created.

## Session contents

A session stores saved document locations only.

Untitled documents are not included until they have been saved and have a
location.

The order of file URIs represents the intended tab order.

Cursor positions, selections, scroll positions, and other view state are not
stored.

## Automatic restoration

After the plugin is activated for a window, restoration of `Previous Session`
is scheduled with an idle callback.

If `Previous Session` cannot be restored, the failure is written as a
warning. The plugin does not display a modal error dialog for this automatic
restoration.

## Named sessions

A named session is created from the saved documents currently open in the
window.

An empty session is not saved. If the current window contains no saved
documents, Session Saver displays an informational message.

The name `Previous Session` is reserved and cannot be used for a named
session.

Named session names are compared case-insensitively for ASCII letters.
Therefore, names such as `Work`, `work`, and `WORK` are treated as the same
session name.

Creating a new named session with an existing name is not allowed.

When renaming an existing session, changing only the letter case of its own
name is allowed, for example from `WorK` to `Work`.

## Switching sessions

Restoring a named session replaces the documents currently open in the
window with the documents stored in the selected session.

The replacement is performed asynchronously.

If the operation fails or is cancelled, Session Saver displays a warning
that the session was not restored. Additional error details may also be
provided by the underlying save or restore operation.

## Managing named sessions

The management dialog supports:

- restoring a selected session;
- renaming a selected session;
- deleting a selected session after confirmation.

The following shortcuts are supported while the management dialog is open:

- `Enter`: restore the selected session;
- double-click: restore the selected session;
- `F2`: rename the selected session.

After a named session is saved, renamed, or deleted, the application menu is
refreshed so that the available session entries reflect the current data.

## Save and error behavior

When Session Saver writes `saved-sessions.xml`, an error is reported if the
file cannot be written.

Automatic `Previous Session` updates report write failures as warnings.

Named-session operations display an error dialog when saving, renaming, or
deleting fails.

The exact handling of modified documents during session replacement is
implemented by the asynchronous replacement service and is outside the XML
format itself.
