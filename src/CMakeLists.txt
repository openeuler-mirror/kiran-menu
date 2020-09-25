cmake_minimum_required(VERSION 3.0)

add_executable(kiran-applet
        kiran-menu-app-launcher.cpp
        kiran-menu-app-item.cpp
        kiran-menu-category-item.cpp
	kiranhelper.cpp
        kiran-menu-applet-button.cpp
        kiran-menu-list-item.cpp
        kiran-menu-profile.cpp
        kiran-menu-window.cpp
        kiran-menu-power-button.cpp
        kiranpower.cpp
        kiran-menu-power-menu.cpp
        kiran-search-entry.cpp
        kiran-user-info.cpp
        kiran-menu-avatar-widget.cpp
	applet.cpp
        kiran-tasklist-app-button.cpp
	kiran-tasklist-app-previewer.cpp
	kiran-tasklist-window-previewer.cpp
        kiran-tasklist-widget.cpp
        kiran-opacity-menu.cpp
        kiran-app-context-menu.cpp
        kiran-window-context-menu.cpp
        kiran-window-thumbnail.cpp
        workspace-thumbnail.cpp
        workspace-window-thumbnail.cpp
        workspace-applet-window.cpp
        workspace-windows-overview.cpp
        window_arrange.cpp
        kiran-applet-button.cpp
        workspace-applet-button.cpp
        workarea-monitor.cpp
)

install(TARGETS kiran-applet DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/mate-panel/")

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)
pkg_search_module(MATEDESKTOP REQUIRED mate-desktop-2.0)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
