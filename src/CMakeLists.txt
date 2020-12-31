cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-applet)

add_executable(${TARGET_NAME}
        kiran-search-entry.cpp
        kiran-opacity-menu.cpp
        kiran-applet-button.cpp
        kiran-thumbnail-widget.cpp
        window-thumbnail-widget.cpp
        menu-user-info.cpp
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
        menu-apps-container.cpp
        menu-new-apps-container.cpp
        tasklist-app-button.cpp
        tasklist-app-previewer.cpp
        tasklist-window-previewer.cpp
        tasklist-applet-widget.cpp
        tasklist-buttons-container.cpp
        tasklist-app-context-menu.cpp
        tasklist-window-context-menu.cpp
        tasklist-paging-button.cpp
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


include_directories(${CMAKE_BINARY_DIR})

target_compile_options(${TARGET_NAME} PUBLIC "-L${PROJECT_BINARY_DIR}/lib" -DG_LOG_DOMAIN=\"${TARGET_NAME}\")
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${MATEDESKTOP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${ZLOG_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(${TARGET_NAME} PUBLIC "${MATEPANEL_LIBRARIES}")
target_link_libraries(${TARGET_NAME} PUBLIC "${GTKMM_LIBRARIES}")
target_link_libraries(${TARGET_NAME} PUBLIC "${MATEDESKTOP_LIBRARIES}")
target_link_libraries(${TARGET_NAME} PUBLIC "${ACCOUNTSSERVICE_LIBRARIES}")
target_link_libraries(${TARGET_NAME} PRIVATE kiran-menu-backend)
target_link_libraries(${TARGET_NAME} PUBLIC "X11")
target_link_libraries(${TARGET_NAME} PUBLIC "${ZLOG_LIBRARIES}")
