shared_module(
  'sessionmanager',
  sources: files(
    'gedit-sessionsaver-app-activatable.c',
    'gedit-sessionsaver-window-activatable.c',
    'sessionsaver-i18n.c',
    'sessionsaver-manage-dialog.c',
    'sessionsaver-save-dialog.c',
    'sessionsaver-service.c',
    'sessionsaver-session.c',
    'sessionsaver-storage.c',
    'sessionsaver-window.c',
    'sessionsaver-xml.c',
  ),
  include_directories: root_include_dir,
  dependencies: libgedit_dep,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
  name_suffix: module_suffix,
)
custom_target(
  'sessionmanager.plugin',
  input: 'sessionsaver.plugin.desktop.in',
  output: 'sessionmanager.plugin',
  command: msgfmt_plugin_cmd,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
)