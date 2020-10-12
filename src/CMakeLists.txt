cmake_minimum_required(VERSION 3.0)

add_executable(kiran-applet
        kiran-search-entry.cpp
        kiran-user-info.cpp
        kiran-opacity-menu.cpp
        kiran-applet-button.cpp
        window-thumbnail-widget.cpp
        menu-list-item-widget.cpp
        menu-app-launcher-button.cpp
        menu-app-item.cpp
        menu-category-item.cpp
        menu-applet-button.cpp
        menu-profile.cpp
        menu-applet-window.cpp
        menu-power-button.cpp
        menu-power-menu.cpp
        menu-avatar-widget.cpp
        tasklist-app-button.cpp
        tasklist-app-previewer.cpp
        tasklist-window-previewer.cpp
        tasklist-applet-widget.cpp
        tasklist-buttons-container.cpp
        tasklist-app-context-menu.cpp
        tasklist-window-context-menu.cpp
        workspace-thumbnail.cpp
        workspace-window-thumbnail.cpp
        workspace-applet-window.cpp
        workspace-windows-overview.cpp
        window_arrange.cpp
        workspace-applet-button.cpp
        workarea-monitor.cpp
        kiran-helper.cpp
        kiran-power.cpp
        applet.cpp
)

install(TARGETS kiran-applet DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/mate-panel/")

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)
pkg_search_module(MATEDESKTOP REQUIRED mate-desktop-2.0)


include_directories(${CMAKE_BINARY_DIR})

target_compile_options(kiran-applet PUBLIC "-std=c++11" "-g" "-L${PROJECT_BINARY_DIR}/lib")
target_include_directories(kiran-applet PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${MATEDESKTOP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(kiran-applet PUBLIC "${MATEPANEL_LIBRARIES}")
target_link_libraries(kiran-applet PUBLIC "${GTKMM_LIBRARIES}")
target_link_libraries(kiran-applet PUBLIC "${MATEDESKTOP_LIBRARIES}")
target_link_libraries(kiran-applet PUBLIC "${ACCOUNTSSERVICE_LIBRARIES}")
target_link_libraries(kiran-applet PRIVATE kiran-menu-backend)
target_link_libraries(kiran-applet PUBLIC "X11")
