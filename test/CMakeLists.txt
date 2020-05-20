cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

pkg_search_module(GTK3 REQUIRED gtk+-3.0)

set(DEP_LIBDIR ${GTK3_LIBDIR})


set(TEST_MENU_UNIT test-menu-unit)

add_executable(${TEST_MENU_UNIT}
    "test-all-apps.c"
    "test-app-category.c"
    "test-app-favorite.c"
    "test-app-frequent.c"
    "test-app-search.c"
    "test-menu-unit.c")

target_include_directories(${TEST_MENU_UNIT} PRIVATE
    ${PROJECT_SOURCE_DIR})

target_include_directories(${TEST_MENU_UNIT} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${GTK3_INCLUDE_DIRS})

target_compile_options(${TEST_MENU_UNIT} PRIVATE
    -L${DEP_LIBDIR}
    -std=c99)

target_link_libraries(${TEST_MENU_UNIT} PRIVATE
    kiran-menu-backend
    ${GTK3_LIBRARIES})


set(TEST_MENU_SIGNAL test-menu-signal)

add_executable(${TEST_MENU_SIGNAL}
    "test-menu-signal.c")

target_include_directories(${TEST_MENU_SIGNAL} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${GTK3_INCLUDE_DIRS})

target_compile_options(${TEST_MENU_SIGNAL} PRIVATE
    -L${DEP_LIBDIR}
    -std=c99)

target_link_libraries(${TEST_MENU_SIGNAL} PRIVATE
    kiran-menu-backend
    ${GTK3_LIBRARIES})


add_test(menu_test ${TEST_MENU_UNIT})
add_test(menu_test ${TEST_MENU_SIGNAL})
