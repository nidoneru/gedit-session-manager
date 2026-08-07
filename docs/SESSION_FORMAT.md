# Session Manager XML Format

## File name

Session Manager stores its data in:

    saved-sessions.xml

## Format written by Session Manager

Session Manager writes an XML document with an XML declaration and one
`saved-sessions` root element.

Example:

    <?xml version="1.0" encoding="UTF-8"?>
    <saved-sessions>
    </saved-sessions>

## Session element

Each `session` element represents one saved session.

Example:

    <session name="work">
    </session>

The `name` attribute is required and must not be empty.

Session names are expected to be unique. Normal Session Manager operations
preserve this condition, but the XML parser does not reject duplicate names
in a manually edited file.

For normal Session Manager operations, session names are compared
case-insensitively for ASCII letters. For example, `Work`, `work`, and
`WORK` are treated as the same session name.

The session name is escaped when the XML file is written.

## File element

Each `file` element represents one document in the session.

Example:

    <file path="file:///home/user/Documents/example.txt"/>

The `path` attribute is required and must not be empty.

Despite its name, its value is a GIO URI, not a native filesystem path.

A `file` element must appear inside a `session` element.

The file URI is escaped when the XML file is written.

The order of `file` elements represents the intended tab order.

## Reserved session

The session name `Previous Session` is reserved.

It stores the most recently captured set of open saved documents.

Users cannot create, rename, or delete a named session using this reserved
name.

## Minimal example

    <?xml version="1.0" encoding="UTF-8"?>
    <saved-sessions>
      <session name="Previous Session">
        <file path="file:///home/user/Documents/example1.txt"/>
        <file path="file:///home/user/Documents/example2.txt"/>
      </session>
      <session name="work">
        <file path="file:///home/user/Documents/work.txt"/>
      </session>
    </saved-sessions>

## Parser behavior

The current parser accepts the following elements:

- `saved-sessions`
- `session`
- `file`

Unknown XML elements cause a loading error.

Unknown attributes are ignored.

Loading also fails when:

- a `session` element has no valid `name` attribute;
- a `file` element appears outside a `session` element;
- a `file` element has no valid `path` attribute;
- the XML is not well formed.

The parser does not currently enforce all structural rules of the format.
For example, it does not explicitly verify that there is exactly one
`saved-sessions` root element, and it does not reject duplicate session
names.

## Compatibility policy

The format follows the legacy gedit Session Saver structure:

- `saved-sessions`
- `session`
- `name`
- `file`
- `path`

No format-version element or attribute is currently written.
