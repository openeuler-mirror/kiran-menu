cmake_minimum_required(VERSION 3.0)

configure_file(org.mate.panel.KiranApplet.mate-panel-applet.in
               org.mate.panel.KiranApplet.mate-panel-applet)
configure_file(org.mate.panel.applet.KiranAppletFactory.service.in
               org.mate.panel.applet.KiranAppletFactory.service)
configure_file(org.kiran.panel.applet.KiranAppletFactory.service.in
               org.kiran.panel.applet.KiranAppletFactory.service)

message("-- Checking for program 'glib-compile-resources'")
find_program(COMPILE_RESOURCE NAMES "glib-compile-resources")

add_custom_target(
  kiran-applet.gresource ALL
  ${COMPILE_RESOURCE} ${PROJECT_SOURCE_DIR}/data/applet.gresource.xml --target
  kiran-applet.gresource --sourcedir ${PROJECT_SOURCE_DIR})

install(FILES ${PROJECT_BINARY_DIR}/data/kiran-applet.gresource
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

install(
  FILES ${PROJECT_BINARY_DIR}/data/org.mate.panel.KiranApplet.mate-panel-applet
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${PANEL_NAME}/applets")

install(
  FILES
    ${PROJECT_BINARY_DIR}/data/org.kiran.panel.applet.KiranAppletFactory.service
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/dbus-1/services")

install(
  FILES "com.kylinsec.kiran.startmenu.gschema.xml.in"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
  RENAME "com.kylinsec.kiran.startmenu.gschema.xml")

install(FILES "com.kylinsec.Kiran.MenuCategory.xml"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/)

install(
  FILES "com.kylinsec.kiran.taskbar.gschema.xml.in"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
  RENAME "com.kylinsec.kiran.taskbar.gschema.xml")

install(
  FILES "com.kylinsec.kiran.workspace-switcher.gschema.xml.in"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
  RENAME "com.kylinsec.kiran.workspace-switcher.gschema.xml")

install(
  FILES "com.kylinsec.kiran.tray.gschema.xml.in"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
  RENAME "com.kylinsec.kiran.tray.gschema.xml")

install(FILES "menu-menu.ui.xml" "tasklist-menu.ui.xml"
              "workspace-switcher-menu.ui.xml"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")

# install(CODE "execute_process(COMMAND glib-compile-schemas
# \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")

# pc file
file(GLOB PC_IN_FILES ./*.pc.in)

foreach(PC_IN_FILE IN LISTS PC_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" PC_FILE ${PC_IN_FILE})
  configure_file(${PC_IN_FILE} ${PROJECT_BINARY_DIR}/data/${PC_FILE})

  install(FILES ${PROJECT_BINARY_DIR}/data/${PC_FILE}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endforeach()
