project('kiran-menu-applet', 'c', meson_version: '>=0.47')

appletdep = dependency('libmatepanelapplet-4.0')
executable('kiran-menu-applet', ['kiran-menu-applet-button.c', 'applet.c'],
        dependencies: appletdep,
        install: true,
        install_dir: 'libexec/mate-panel/')

sed = find_program('sed', '/usr/bin/sed', required: true)
custom_target('dbus-services', 
        build_by_default: true,
        input: 'data/org.mate.panel.applet.KiranMenuAppletFactory.service.in',
        output: 'org.mate.panel.applet.KiranMenuAppletFactory.service',
        command: [sed, 's:@INSTALL_PREFIX@:' + get_option('prefix') + ':g', '@INPUT@'],
        capture: true,
        install: true,
        install_mode: 'rw-r--r--',
        install_dir: 'share/dbus-1/services/')

custom_target('applet-services', 
        build_by_default: true,
        input: 'data/org.mate.panel.KiranMenuApplet.mate-panel-applet.in',
        output: 'org.mate.panel.KiranMenuApplet.mate-panel-applet',
        command: [sed, 's:@INSTALL_PREFIX@:' + get_option('prefix') + ':g', '@INPUT@'],
        capture: true,
        install: true,
        install_mode: 'rw-r--r--',
        install_dir: 'share/mate-panel/applets/')
