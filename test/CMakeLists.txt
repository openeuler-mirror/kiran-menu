cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

pkg_search_module(GTK3 REQUIRED gtk+-3.0)

set(DEP_LIBDIR ${GTK3_LIBDIR})


set(TARGET_NAME test-start-menu)

file(GLOB_RECURSE MENU_TEST_H_FILES ./*.h)
file(GLOB_RECURSE MENU_TEST_C_FILES ./*.c)

set (ALL_FILES ${MENU_TEST_H_FILES} ${MENU_TEST_C_FILES})


add_executable(${TARGET_NAME} ${ALL_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${GTK3_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PRIVATE
    -L${DEP_LIBDIR}
    -std=c99)

target_link_libraries(${TARGET_NAME} PRIVATE
    kiran-menu-backend
    ${GTK3_LIBRARIES})


add_test(menu_test ${TARGET_NAME})
