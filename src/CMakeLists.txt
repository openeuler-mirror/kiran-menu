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
        menu/recent-files-list-box.cpp
        menu/recent-files-widget.cpp
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
        showdesktop/showdesktop-applet-button.cpp
        showdesktop/showdesktop-applet.cpp
        applet.cpp
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

if(BUILD_WITH_KIRANACCOUNTS)
target_sources(${TARGET_NAME} PUBLIC menu/kiran-accounts-user.cpp menu/kiran-accounts-manager.cpp)
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

target_compile_options(${TARGET_NAME} PUBLIC -g "-L${PROJECT_BINARY_DIR}/lib" -DG_LOG_DOMAIN=\"${TARGET_NAME}\")
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${MATEDESKTOP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${ZLOG_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_SOURCE_DIR}/src/common"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(${TARGET_NAME} PUBLIC "${MATEPANEL_LIBRARIES}"
            "${GTKMM_LIBRARIES}"
            "${MATEDESKTOP_LIBRARIES}"
            "${ACCOUNTSSERVICE_LIBRARIES}"
            "${ZLOG_LIBRARIES}"
            "X11")

target_link_libraries(${TARGET_NAME} PRIVATE kiran-menu-backend)
