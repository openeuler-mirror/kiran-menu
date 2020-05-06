cmake_minimum_required(VERSION 3.0)

add_executable(kiran-menu-applet applet.c
            kiran-app-button.c
            kiran-menu-applet-button.c
            kiran-menu-window.c
            kiran-power-button.c
            kiran-power-menu.c
            kiran-search-entry.c
)

pkg_search_module(MATEPANEL REQUIRED libmatepanelapplet-4.0)

configure_file(config.h.in config.h)

target_compile_options(kiran-menu-applet PUBLIC "-std=c99" "-L${PROJECT_BINARY_DIR}/lib")
target_include_directories(kiran-menu-applet PUBLIC "${PROJECT_SOURCE_DIR}/lib"
            "${MATEPANEL_INCLUDE_DIRS}"
            "${PROJECT_BINARY_DIR}/src")

target_link_libraries(kiran-menu-applet PUBLIC "${MATEPANEL_LIBRARIES}")
target_link_libraries(kiran-menu-applet PRIVATE "${PROJECT_BINARY_DIR}/lib/lib${PROJECT_NAME}.a")
