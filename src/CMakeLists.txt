cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-applet)

file (GLOB_RECURSE SRC_H_FILES ./*.h)
file (GLOB_RECURSE SRC_CPP_FILES ./*.cpp)
file (GLOB_RECURSE SRC_C_FILES ./*.c)

add_executable(${TARGET_NAME}
               ${SRC_H_FILES}
               ${SRC_CPP_FILES}
               ${SRC_C_FILES}
               kiran-sn-host-gen.c
               kiran-sn-host-gen.h
               kiran-sn-item-gen.c
               kiran-sn-item-gen.h
               kiran-sn-watcher-gen.c
               kiran-sn-watcher-gen.h
	       kiran-sn-manager-gen.c
	       kiran-sn-manager-gen.h
	       )


if(BUILD_WITH_KIRANACCOUNTS)
    target_sources(${TARGET_NAME} PUBLIC
                   ${PROJECT_SOURCE_DIR}/src/menu/kiran-accounts-user.cpp
                   ${PROJECT_SOURCE_DIR}/src/menu/kiran-accounts-manager.cpp)
    pkg_search_module(ACCOUNTSSERVICE REQUIRED kiran-cc-daemon)
else()
    pkg_search_module(ACCOUNTSSERVICE REQUIRED accountsservice)
endif()

install(TARGETS ${TARGET_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/${PANEL_NAME}/")

include(GNUInstallDirs)
install(FILES tray/tray-i.h  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/")

include(CheckIncludeFiles)


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

add_custom_command(OUTPUT kiran-sn-manager-gen.c kiran-sn-manager-gen.h
	COMMAND ${GDBUS_CODEGEN} --c-namespace KiranSn --annotate "org.kde.StatusNotifierManager" org.gtk.GDBus.C.Name ManagerGen  --generate-c-code kiran-sn-manager-gen  ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierManager.xml
	DEPENDS ${PROJECT_SOURCE_DIR}/data/org.kde.StatusNotifierManager.xml
)

include_directories(${CMAKE_BINARY_DIR})

target_compile_options(${TARGET_NAME} PUBLIC -g "-L${PROJECT_BINARY_DIR}/lib" -DG_LOG_DOMAIN=\"${TARGET_NAME}\")
target_include_directories(${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${PANEL_INCLUDE_DIRS}"
            "${GTKMM_INCLUDE_DIRS}"
            "${MATEDESKTOP_INCLUDE_DIRS}"
            "${ACCOUNTSSERVICE_INCLUDE_DIRS}"
            "${KLOG_GTK3_INCLUDE_DIRS}"
            "${GTK3_INCLUDE_DIRS}"
            "${DBUSMENU_GLIB_INCLUDE_DIRS}"
            "${LIBXML2_INCLUDE_DIRS}"
            "${GLIB_JSON_INCLUDE_DIRS}"
            "${GOBJECT2_INCLUDE_DIRS}"
            "${GLIB2_INCLUDE_DIRS}"
            "${FMT_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}"
            "${PROJECT_SOURCE_DIR}/src/common"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(${TARGET_NAME} PUBLIC "${PANEL_LIBRARIES}"
            "${GTKMM_LIBRARIES}"
            "${MATEDESKTOP_LIBRARIES}"
            "${ACCOUNTSSERVICE_LIBRARIES}"
            "${DBUSMENU_GLIB_LIBRARIES}"
            "${LIBXML2_LIBRARIES}"
            "${GLIB_JSON_LIBRARIES}"
            "${GOBJECT2_LIBRARIES}"
            "${GLIB2_LIBRARIES}"
            "${KLOG_GTK3_LIBRARIES}"
            "${FMT_LIBRARIES}"
            "X11"
            "m")

target_link_libraries(${TARGET_NAME} PRIVATE kiran-menu-backend)
