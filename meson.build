shared_module(
  'sessionmanager',
  sources: files(
    'gedit-sessionmanager-app-activatable.c',
    'gedit-sessionmanager-window-activatable.c',
    'sessionmanager-i18n.c',
    'sessionmanager-manage-dialog.c',
    'sessionmanager-save-dialog.c',
    'sessionmanager-service.c',
    'sessionmanager-session.c',
    'sessionmanager-storage.c',
    'sessionmanager-window.c',
    'sessionmanager-xml.c',
  ),
  include_directories: root_include_dir,
  dependencies: libgedit_dep,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
)
custom_target(
  'sessionmanager.plugin',
  input: 'sessionmanager.plugin.desktop.in',
  output: 'sessionmanager.plugin',
  command: msgfmt_plugin_cmd,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
)