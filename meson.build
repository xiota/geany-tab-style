project(
  'geany-xi-tweaks',
  'cpp',
  license : 'GPL-3.0-or-later',
  default_options : ['buildtype=release', 'prefix=/usr']
)

prefix = get_option('prefix')

plugin_name = 'xi-tweaks'
project_datadir = get_option('datadir') / meson.project_name()
project_docdir = get_option('datadir') / 'doc' / meson.project_name()
plugin_conf = plugin_name + '.conf'

geany = dependency('geany')

conf_data = configuration_data()
conf_data.set('version', meson.project_version())

conf_data.set('prefix', prefix)
conf_data.set('project_datadir', project_datadir)

conf_data.set('plugin_conf', plugin_conf)

config_h = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: conf_data,
)

library(
  plugin_name,
  sources: [
    config_h,
    'source/auxiliary.cc',
    'source/plugin.cc',
    'source/prefs.cc',
  ],
  dependencies: [geany],
  name_prefix: '',
  install: true,
  install_dir: get_option('libdir') / 'geany',
)
