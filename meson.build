project(
  'kiran-start-menu', 'c',
        version : '3.3.0',
  meson_version : '>= 0.50.0'
)

gnome = import('gnome')


start_menu_prefix = get_option('prefix')
start_menu_libexecdir = join_paths(start_menu_prefix, get_option('libexecdir'))
start_menu_datadir = join_paths(start_menu_prefix, get_option('datadir'))
start_menu_localedir = join_paths(start_menu_prefix, get_option('localedir'))
start_menu_schemadir = join_paths(start_menu_datadir, 'glib-2.0', 'schemas')
top_inc = include_directories('.')

gio_dep = dependency('gio-2.0')
glib_dep = dependency('glib-2.0', version: '>= 2.53.0')

common_deps = [
  gio_dep,
  glib_dep,
  dependency('gio-unix-2.0'),
  dependency('gthread-2.0'),
]


subdir('src')
subdir('test')

