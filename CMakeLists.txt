
cmake_minimum_required(VERSION 3.0)

project(kiran-menu VERSION 2.0)


find_package(PkgConfig)

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)


set(DEP_INCLUDE_DIRS 
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS} 
    ${GIO_UNIX_INCLUDE_DIRS}  
    ${GTHREAD_INCLUDE_DIRS})

set(DEP_LIBRARIES
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
    ${GTHREAD_LIBRARIES})

set(DEP_LIBDIR ${GLIB_LIBDIR})


add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(test/start-menu)

