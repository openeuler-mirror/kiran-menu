cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-applet)

add_executable(${TARGET_NAME}
        common/kiran-helper.cpp
        common/kiran-power.cpp
        common/kiran-search-entry.cpp
        common/kiran-opacity-menu.cpp
        common/kiran-applet-button.cpp
        common/kiran-thumbnail-widget.cpp
        common/window-thumbnail-widget.cpp
        common/workarea-monitor.cpp
        menu/menu-user-info.cpp
        menu/menu-list-item-widget.cpp
        menu/menu-app-launcher-button.cpp
        menu/menu-app-item.cpp
        menu/menu-category-item.cpp
        menu/menu-applet-button.cpp
        menu/menu-profile.cpp
        menu/menu-applet-window.cpp
        menu/menu-power-button.cpp
        menu/menu-power-menu.cpp
        menu/menu-avatar-widget.cpp
        menu/menu-apps-container.cpp
        menu/menu-new-apps-container.cpp
        menu/menu-applet.cpp
        tasklist/tasklist-app-button.cpp
        tasklist/tasklist-app-previewer.cpp
        tasklist/tasklist-window-previewer.cpp
        tasklist/tasklist-applet-widget.cpp
        tasklist/tasklist-buttons-container.cpp
        tasklist/tasklist-app-context-menu.cpp
        tasklist/tasklist-window-context-menu.cpp
        tasklist/tasklist-paging-button.cpp
        tasklist/tasklist-applet.cpp
        workspace/workspace-thumbnail.cpp
        workspace/workspace-window-thumbnail.cpp
        workspace/workspace-applet-window.cpp
        workspace/workspace-windows-overview.cpp
        workspace/window_arrange.cpp
        workspace/workspace-applet-button.cpp
        workspace/workspace-applet.cpp
        tray/kiran-notify-icon.c
        tray/kiran-notify-icon.h
        tray/kiran-notify-icon-window.c
        tray/kiran-notify-icon-window.h
        tray/kiran-sn-icon.c
        tray/kiran-sn-icon.h
        tray/kiran-sn-icon-menu.c
        tray/kiran-sn-icon-menu.h
        tray/kiran-sn-tray-manager.c
        tray/kiran-sn-tray-manager.h
        tray/kiran-sn-watcher.c
        tray/kiran-sn-watcher.h
        tray/kiran-tray-applet.c
        tray/kiran-tray.c
        tray/kiran-tray.h
        tray/kiran-tray-manager.c
        tray/kiran-tray-manager.h
        tray/kiran-x11-tray-icon.cpp
        tray/kiran-x11-tray-icon.h
        tray/kiran-x11-tray-manager.c
        tray/kiran-x11-tray-manager.h
        tray/kiran-x11-tray-socket.c
        tray/kiran-x11-tray-socket.h
        kiran-sn-host-gen.c
        kiran-sn-item-gen.c
        kiran-sn-watcher-gen.c
        applet.cpp
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

if(BUILD_WITH_KIRANACCOUNTS)
target_sources(${TARGET_NAME} PUBLIC kiran-accounts-user.c kiran-accounts-manager.c)
pkg_search_module(ACCOUNTSSERVICE REQUIRED kiran-cc-daemon)
else()
pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)
endif()
install(TARGETS ${TARGET_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/mate-panel/")

include(CheckIncludeFiles)

CHECK_INCLUDE_FILES(zlog_ex.h HAVE_ZLOG_HEADERS)
if (NOT HAVE_ZLOG_HEADERS)
    message(FATAL_ERROR "zlog_ex.h not found")
endif (NOT HAVE_ZLOG_HEADERS)

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(MATEDESKTOP REQUIRED mate-desktop-2.0)
pkg_search_module(ZLOG REQUIRED zlog)

pkg_search_module(GLIB2 REQUIRED glib-2.0)
pkg_search_module(GOBJECT2 REQUIRED gobject-2.0)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)
pkg_search_module(DBUSMENU_GLIB REQUIRED dbusmenu-glib-0.4)
pkg_search_module(LIBXML2 REQUIRED libxml-2.0)

find_library(LIBX11 X11)
find_library(LIBM m)
find_program(GDBUS_CODEGEN "gdbus-codegen")

if (NOT GDBUS_CODEGEN)
message(FATAL_ERROR "gdbus-codegen program not found")
else()
message("found ${GDBUS_CODEGEN}")
endif (NOT GDBUS_CODEGEN)

add_custom_command(OUTPUT kiran-sn-host-gen.c kiran-sn-host-gen.h
    COMMAND ${GDBUS_CODEGEN} --c-namespace KiranSn --generate-c-code kiran-sn-host-gen  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierHost.xml
    DEPENDS ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierHost.xml
)

add_custom_command(OUTPUT kiran-sn-item-gen.c kiran-sn-item-gen.h
    COMMAND ${GDBUS_CODEGEN} --c-namespace KiranSn --generate-c-code kiran-sn-item-gen  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierItem.xml
    DEPENDS  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierWatcher.xml
)

add_custom_command(OUTPUT kiran-sn-watcher-gen.c kiran-sn-watcher-gen.h
    COMMAND ${GDBUS_CODEGEN} --c-namespace KiranSn --generate-c-code kiran-sn-watcher-gen  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierWatcher.xml
    DEPENDS  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierWatcher.xml
)

include_directories(${CMAKE_BINARY_DIR})

target_compile_options(${TARGET_NAME} PUBLIC "-L${PROJECT_BINARY_DIR}/lib" -DG_LOG_DOMAIN=\"${TARGET_NAME}\")
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${MATEDESKTOP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${ZLOG_INCLUDE_DIRS}"
            "${GTK3_INCLUDE_DIRS}"
            "${DBUSMENU_GLIB_INCLUDE_DIRS}"
            "${LIBXML2_INCLUDE_DIRS}"
            "${GOBJECT2_INCLUDE_DIRS}"
            "${GLIB2_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_SOURCE_DIR}/src/common"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(${TARGET_NAME} PUBLIC "${MATEPANEL_LIBRARIES}"
            "${GTKMM_LIBRARIES}"
            "${MATEDESKTOP_LIBRARIES}"
            "${ACCOUNTSSERVICE_LIBRARIES}"
            "${DBUSMENU_GLIB_LIBRARIES}"
            "${LIBXML2_LIBRARIES}"
            "${GOBJECT2_LIBRARIES}"
            "${GLIB2_LIBRARIES}"
            "${ZLOG_LIBRARIES}"
            "X11"
            "m")

target_link_libraries(${TARGET_NAME} PRIVATE kiran-menu-backend)
