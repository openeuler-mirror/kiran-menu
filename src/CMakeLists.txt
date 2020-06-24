cmake_minimum_required(VERSION 3.0)

add_executable(kiran-menu-applet
	kiranappbutton.cpp
	kiranappitem.cpp
	kirancategoryitem.cpp
	kiranhelper.cpp
	kiranmenuappletbutton.cpp
	kiranmenulistitem.cpp
	kiranmenuprofile.cpp
	kiranmenuwindow.cpp
	kiranpowerbutton.cpp
	kiranpower.cpp
	kiranpowermenu.cpp
	kiransearchentry.cpp
	kiranuserinfo.cpp
	kiranweatherproxy.cpp
	applet.cpp
        kiran-tasklist-app-button.cpp
	kiran-tasklist-app-previewer.cpp
	kiran-tasklist-window-previewer.cpp
        kiran-tasklist-widget.cpp
)

install(TARGETS kiran-menu-applet DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/mate-panel/")

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)


configure_file(config.h.in config.h)

target_compile_options(kiran-menu-applet PUBLIC "-std=c++11" "-g" "-L${PROJECT_BINARY_DIR}/lib")
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
