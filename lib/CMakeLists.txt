cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-menu-backend)

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)


set (ALL_FILES ${CORE_H_FILES} ${CORE_CPP_FILES})

add_library(${TARGET_NAME} ${ALL_FILES})

pkg_search_module(GLIBMM REQUIRED glibmm-2.4)
pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(WNCK3 REQUIRED libwnck-3.0)
pkg_search_module(GDKMM3 REQUIRED gdkmm-3.0)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XCOMPOSITE REQUIRED xcomposite)


set(DEP_LIBDIR ${GLIBMM_LIBDIR})


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${GTHREAD_INCLUDE_DIRS}
    ${WNCK3_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${XCOMPOSITE_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PUBLIC
    -std=c++11
    -DWNCK_I_KNOW_THIS_IS_UNSTABLE)

target_compile_options(${TARGET_NAME} PRIVATE
    -L${DEP_LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GTHREAD_LIBRARIES}
    ${WNCK3_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    ${X11_LIBRARIES}
    ${XCOMPOSITE_LIBRARIES})

