cmake_minimum_required(VERSION 3.0)

configure_file(org.mate.panel.KiranMenuApplet.mate-panel-applet.in org.mate.panel.KiranMenuApplet.mate-panel-applet)
configure_file(org.mate.panel.applet.KiranMenuAppletFactory.service.in org.mate.panel.applet.KiranMenuAppletFactory.service)

message("-- Checking for program 'glib-compile-resources'")
find_program(COMPILE_RESOURCE NAMES "glib-compile-resources")

add_custom_target(menu.gresource ALL 
    ${COMPILE_RESOURCE} ${PROJECT_SOURCE_DIR}/data/menu.gresource.xml --target menu.gresource --sourcedir ${PROJECT_SOURCE_DIR})

install(FILES ${PROJECT_BINARY_DIR}/data/menu.gresource
	DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

install(FILES ${PROJECT_BINARY_DIR}/data/org.mate.panel.KiranMenuApplet.mate-panel-applet
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mate-panel/applets")
    
install(FILES ${PROJECT_BINARY_DIR}/data/org.mate.panel.applet.KiranMenuAppletFactory.service
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/dbus-1/services")
