
cmake_minimum_required(VERSION 3.0)

project(kiran-applet VERSION 2.2.0)

option(BUILD_WITH_ACCOUNTSSERVICE "Build with accountsservice" ON)
option(BUILD_WITH_KIRANACCOUNTS "Build with kiran session daemon" OFF)
option(KIRAN_DESKTOP "Build base kiran desktop environment" OFF)

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

if (KIRAN_DESKTOP)
    set (PANEL_NAME "kiran-panel")
    pkg_search_module(PANEL REQUIRED libkiranpanelapplet-4.0)
else ()
    set (PANEL_NAME "mate-panel")
    pkg_search_module(PANEL REQUIRED libmatepanelapplet-4.0)
endif()

pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(MATEDESKTOP REQUIRED mate-desktop-2.0)
pkg_search_module(KLOG_GTK3 REQUIRED klog-gtk3)

pkg_search_module(GLIB2 REQUIRED glib-2.0)
pkg_search_module(GOBJECT2 REQUIRED gobject-2.0)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)
pkg_search_module(DBUSMENU_GLIB REQUIRED dbusmenu-glib-0.4)
pkg_search_module(LIBXML2 REQUIRED libxml-2.0)
pkg_search_module(FMT REQUIRED fmt)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(test)
add_subdirectory(icons)

