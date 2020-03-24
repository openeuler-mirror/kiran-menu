gnome = import('gnome')

service_conf = configuration_data()
service_conf.set('libexecdir', libexecdir)

service = 'com.unikylin.Kiran.StartMenu.service'

configure_file(
          input : service + '.in',
         output : service,
        install : false,
    install_dir : join_paths(datadir, 'dbus-1', 'services'),
  configuration : service_conf
)

#install_data(
#  'gnome-control-center-search-provider.ini',
#  install_dir: join_paths(datadir, 'kiran', 'start-menu')
#)

sources = files(
  'kiran-app-category.c',
  'kiran-app-favorite.c',
  'kiran-app-search.c',
  'kiran-app-system.c',
  'kiran-app-usage.c',
  'kiran-start-menu-app.c',
  'kiran-skeleton.c',
  'kiran-app-info.c'
)

sources += gnome.gdbus_codegen(
  'kiran-start-menu-generated',
  'com.unikylin.StartMenuS.xml',
  interface_prefix : 'com.unikylin.',
         namespace : 'Kiran'
)


#testa = join_paths(meson.build_root(), 'src', 'com.unikylin.Kiran.StartMenu.gschema.xml')
#message(testa)


schemaconf = configuration_data()
schemaconf.set('GETTEXT_PACKAGE', meson.project_name())
schema = configure_file(
  input: 'com.unikylin.Kiran.StartMenu.gschema.xml.in',
  output: 'com.unikylin.Kiran.StartMenu.gschema.xml',
  configuration: schemaconf,
  install_dir: schemadir
)

cflags = '-DKIRANLOCALEDIR="@0@"'.format(localedir)

sub_libs += static_library(
          'start-menu',
              sources : sources,
  include_directories : [ top_inc ],
         dependencies : common_deps,
               c_args : cflags
)

custom_target('compile-schemas',
  input: schema,
  output: 'gschemas.compiled',
  command: [find_program('glib-compile-schemas'), meson.current_build_dir()],
  build_by_default: true)
