gnome = import('gnome')

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

cflags = '-DKIRANLOCALEDIR="@0@"'.format(start_menu_localedir)

executable(
  'kiran-start-menu',
               sources,
  include_directories : top_inc,
         dependencies : common_deps,
               c_args : cflags,
              install : true,
              link_args: '-g',
          install_dir : start_menu_libexecdir
)

custom_target('compile-schemas',
  input: schema,
  output: 'gschemas.compiled',
  command: [find_program('glib-compile-schemas'), meson.current_build_dir()],
  build_by_default: true)
