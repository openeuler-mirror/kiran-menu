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


service_conf = configuration_data()
service_conf.set('libexecdir', start_menu_libexecdir)

service = 'com.unikylin.Kiran.StartMenu.service'

configure_file(
          input : service + '.in',
         output : service,
        install : false,
    install_dir : join_paths(start_menu_datadir, 'dbus-1', 'services'),
  configuration : service_conf
)

#install_data(
#  'gnome-control-center-search-provider.ini',
#  install_dir: join_paths(start_menu_datadir, 'kiran', 'start-menu')
#)

sources = files(
  'main.c',
  'kiran-start-menu-app.c',
  'kiran-start-menu.c'
)

sources += gnome.gdbus_codegen(
  'kiran-start-menu-generated',
  'com.unikylin.StartMenuS.xml',
  interface_prefix : 'com.unikylin.',
         namespace : 'Kiran'
)

schemaconf = configuration_data()
schemaconf.set('GETTEXT_PACKAGE', meson.project_name())
schema = configure_file(
  input: 'com.unikylin.Kiran.StartMenu.gschema.xml.in',
  output: 'com.unikylin.Kiran.StartMenu.gschema.xml',
  configuration: schemaconf,
  install_dir: start_menu_schemadir
)


top_inc = include_directories('.')

gio_dep = dependency('gio-2.0')
glib_dep = dependency('glib-2.0', version: '>= 2.53.0')

common_deps = [
  gio_dep,
  glib_dep,
  dependency('gio-unix-2.0'),
  dependency('gthread-2.0'),
]


cflags = '-DKIRANLOCALEDIR="@0@"'.format(start_menu_localedir)


executable(
  'kiran-start-menu',
               sources,
  include_directories : top_inc,
         dependencies : common_deps,
               c_args : cflags,
              install : true,
          install_dir : start_menu_libexecdir
)

custom_target('compile-schemas',
  input: schema,
  output: 'gschemas.compiled',
  command: [find_program('glib-compile-schemas'), meson.current_build_dir()],
  build_by_default: true)
