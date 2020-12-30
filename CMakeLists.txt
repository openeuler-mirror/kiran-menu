
cmake_minimum_required(VERSION 3.0)

project(kiran-applet VERSION 2.2.0)

option(BUILD_WITH_ACCOUNTSSERVICE "Build with accountsservice" ON)
option(BUILD_WITH_KIRANACCOUNTS "Build with kiran session daemon" OFF)

find_package(PkgConfig)

configure_file(config.h.in config.h)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(test)
add_subdirectory(icons)

