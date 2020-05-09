cmake_minimum_required(VERSION 3.0)


file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_C_FILES ./*.c)


set (ALL_FILES ${CORE_H_FILES} ${CORE_C_FILES})

add_library(${PROJECT_NAME} ${ALL_FILES})

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(WNCK3 REQUIRED libwnck-3.0)


set(DEP_LIBDIR ${GLIB_LIBDIR})


target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${GIO_INCLUDE_DIRS} 
    ${GIO_UNIX_INCLUDE_DIRS}  
    ${GTHREAD_INCLUDE_DIRS}
    ${WNCK3_INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PUBLIC
    -std=c99
    -DWNCK_I_KNOW_THIS_IS_UNSTABLE)

target_compile_options(${PROJECT_NAME} PRIVATE
    -L${DEP_LIBDIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
    ${GTHREAD_LIBRARIES}
    ${WNCK3_LIBRARIES})



install(FILES "com.unikylin.Kiran.StartMenu.gschema.xml.in" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.StartMenu.gschema.xml" 
)

install(FILES "com.unikylin.Kiran.MenuCategory.xml" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/kiran-menu/
)

install(CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")
