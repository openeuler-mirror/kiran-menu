cmake_minimum_required(VERSION 3.0)

add_executable(kiran-menu-applet
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
        workarea-monitor.cpp
	applet.cpp
)

install(TARGETS kiran-menu-applet DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/mate-panel/")

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)


configure_file(config.h.in config.h)

target_compile_options(kiran-menu-applet PUBLIC "-std=c++11" "-L${PROJECT_BINARY_DIR}/lib")
target_include_directories(kiran-menu-applet PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${JSONCPP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(kiran-menu-applet PUBLIC "${MATEPANEL_LIBRARIES}")
target_link_libraries(kiran-menu-applet PUBLIC "${GTKMM_LIBRARIES}")
target_link_libraries(kiran-menu-applet PUBLIC "${JSONCPP_LIBRARIES}")
target_link_libraries(kiran-menu-applet PUBLIC "${ACCOUNTSSERVICE_LIBRARIES}")
target_link_libraries(kiran-menu-applet PRIVATE kiran-menu-backend)
target_link_libraries(kiran-menu-applet PUBLIC "X11")
