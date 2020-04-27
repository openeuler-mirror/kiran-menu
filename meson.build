project('kiran-menu-applet', 'c', version: '2.0', meson_version: '>=0.47')

appletdep = dependency('libmatepanelapplet-4.0')
config_data = configuration_data()
config_data.set('INSTALL_PREFIX', get_option('prefix'))
config_data.set('PACKAGE_NAME', meson.project_name())

config_header = configure_file(
        configuration: config_data,
        input: 'config.h.in',
        output: 'config.h')


sed = find_program('sed', '/usr/bin/sed', required: true)
custom_target('dbus-services', 
        build_by_default: true,
        input: 'data/org.mate.panel.applet.KiranMenuAppletFactory.service.in',
        output: 'org.mate.panel.applet.KiranMenuAppletFactory.service',
        command: [sed, 's:@INSTALL_PREFIX@:' + get_option('prefix') + ':g', '@INPUT@'],
        capture: true,
        install: true,
        install_mode: 'rw-r--r--',
        install_dir: get_option('datadir') + '/dbus-1/services/')

custom_target('applet-services', 
        build_by_default: true,
        input: 'data/org.mate.panel.KiranMenuApplet.mate-panel-applet.in',
        output: 'org.mate.panel.KiranMenuApplet.mate-panel-applet',
        command: [sed, 's:@INSTALL_PREFIX@:' + get_option('prefix') + ':g', '@INPUT@'],
        capture: true,
        install: true,
        install_mode: 'rw-r--r--',
        install_dir: get_option('datadir') + '/mate-panel/applets/')

compile_resource = find_program('glib-compile-resources', '/usr/bin/glib-compile-resources', required: true)
custom_target('resource-binary',
        build_by_default: true,
        build_always_stale: true,
        input: 'data/menu.gresource.xml',
        output: 'menu.gresource',
        command: [compile_resource, '@INPUT@', '--target', '@OUTPUT@', '--sourcedir', '@SOURCE_DIR@'],
        install: true,
        install_mode: 'r--r--r--',
        install_dir: get_option('datadir') + '/' + meson.project_name())


sources=['kiran-menu-applet-button.c',
        'applet.c',
        'kiran-menu-window.c',
        'kiran-search-entry.c',
        'kiran-app-button.c',
        'kiran-power-button.c',
        'kiran-power-menu.c',
        config_header]

executable('kiran-menu-applet', 
        sources,
        dependencies: appletdep,
        link_args: '-rdynamic',
        c_args: '-std=c99',
        install: true,
        install_dir: get_option('libexecdir') + '/mate-panel/')