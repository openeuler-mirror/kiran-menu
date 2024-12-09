cmake_minimum_required(VERSION 3.0)

project(kiran-applet VERSION 2.3)

find_package(PkgConfig)

configure_file(config.h.in config.h)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

pkg_search_module(GLIBMM REQUIRED glibmm-2.4)
pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(WNCK3 REQUIRED libwnck-3.0)
pkg_search_module(GDKMM3 REQUIRED gdkmm-3.0)
pkg_search_module(X11_XCB REQUIRED x11-xcb)
pkg_search_module(XCB_RES REQUIRED xcb-res)
pkg_search_module(XCOMPOSITE REQUIRED xcomposite)

#set(PANEL_NAME "kiran-panel")
#pkg_search_module(PANEL REQUIRED libkiranpanelapplet-4.0)

set(PANEL_NAME "mate-panel")
pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)

include(GNUInstallDirs)
set(KMU_INSTALL_INCLUDE
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(MATEDESKTOP REQUIRED mate-desktop-2.0)
pkg_search_module(KLOG_GTK3 REQUIRED klog-gtk3)

pkg_search_module(GLIB2 REQUIRED glib-2.0)
pkg_search_module(GOBJECT2 REQUIRED gobject-2.0)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)
pkg_search_module(DBUSMENU_GLIB REQUIRED dbusmenu-glib-0.4)
pkg_search_module(LIBXML2 REQUIRED libxml-2.0)
pkg_search_module(FMT REQUIRED fmt)
pkg_search_module(GLIB_JSON REQUIRED json-glib-1.0)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(test)
add_subdirectory(icons)
