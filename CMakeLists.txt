
cmake_minimum_required(VERSION 3.0)

project(kiran-applet VERSION 2.0)


find_package(PkgConfig)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
#add_subdirectory(test)

