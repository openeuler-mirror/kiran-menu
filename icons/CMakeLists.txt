install(FILES ${PROJECT_SOURCE_DIR}/icons/start-menu.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-menu.svg")
install(FILES ${PROJECT_SOURCE_DIR}/icons/window-switcher.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-window-switcher.svg")
install(FILES ${PROJECT_SOURCE_DIR}/icons/workspace-switcher.svg
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/"
  RENAME "kiran-workspace-switcher.svg")

file(GLOB_RECURSE SYMBOLIC_SVG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/kiran-*.svg)
install(FILES ${SYMBOLIC_SVG_FILES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/")

