cmake_minimum_required(VERSION 3.0)

configure_file(org.mate.panel.KiranApplet.mate-panel-applet.in org.mate.panel.KiranApplet.mate-panel-applet)
configure_file(org.mate.panel.applet.KiranAppletFactory.service.in org.mate.panel.applet.KiranAppletFactory.service)

message("-- Checking for program 'glib-compile-resources'")
find_program(COMPILE_RESOURCE NAMES "glib-compile-resources")

add_custom_target(kiran-applet.gresource ALL 
    ${COMPILE_RESOURCE} ${PROJECT_SOURCE_DIR}/data/applet.gresource.xml --target kiran-applet.gresource --sourcedir ${PROJECT_SOURCE_DIR})

install(FILES ${PROJECT_BINARY_DIR}/data/kiran-applet.gresource
	DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

install(FILES ${PROJECT_BINARY_DIR}/data/org.mate.panel.KiranApplet.mate-panel-applet
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mate-panel/applets")
    
install(FILES ${PROJECT_BINARY_DIR}/data/org.mate.panel.applet.KiranAppletFactory.service
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/dbus-1/services")

install(FILES "com.unikylin.Kiran.StartMenu.gschema.xml.in" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.StartMenu.gschema.xml" 
)

install(FILES "com.unikylin.Kiran.StartMenu.profile.gschema.xml" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
)

install(FILES "com.unikylin.Kiran.MenuCategory.xml" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/
)


install(FILES "com.unikylin.Kiran.TaskBar.gschema.xml.in" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.TaskBar.gschema.xml" 
)

install(FILES "com.unikylin.Kiran.WorkspaceSwitcher.gschema.xml.in"
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.WorkspaceSwitcher.gschema.xml"
)

install(FILES "com.unikylin.Kiran.Tray.gschema.xml.in"
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.Tray.gschema.xml"
)

install(FILES "menu-menu.ui.xml" "tasklist-menu.ui.xml" "workspace-switcher-menu.ui.xml"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

#install(CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")
