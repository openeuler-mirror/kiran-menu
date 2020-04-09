project(
  'kiran-menu-backend', 'c',
        version : '3.3.0',
  meson_version : '>= 0.50.0'
)

gnome = import('gnome')


prefix = get_option('prefix')
libexecdir = join_paths(prefix, get_option('libexecdir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
schemadir = join_paths(datadir, 'glib-2.0', 'schemas')
top_inc = include_directories('.')

output = ''
output += '\n        ' + meson.project_name() + ' - ' + meson.project_version() + '\n'
output += '    ===================================\n'
output += '        prefix=' + prefix + '\n'
output += '    libexecdir=' + libexecdir + '\n'
output += '       datadir=' + datadir + '\n'
output += '     localedir=' + localedir + '\n'
output += '     schemadir=' + schemadir + '\n'

message(output)



gio_dep = dependency('gio-2.0')
glib_dep = dependency('glib-2.0', version: '>= 2.53.0')

common_deps = [
  gio_dep,
  glib_dep,
  dependency('gio-unix-2.0'),
  dependency('gthread-2.0'),
]

sub_libs = []

cflags = '-std=c99'


subdir('src')
#subdir('test')

