cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

pkg_search_module(GTKMM REQUIRED gtkmm-3.0)

set(DEP_LIBDIR ${GTKMM_LIBDIR})


set(TEST_MENU_UNIT test-menu-unit)

add_executable(${TEST_MENU_UNIT}
    "test-all-apps.cpp"
    "test-app-category.cpp"
    "test-app-favorite.cpp"
    "test-app-frequent.cpp"
    "test-app-search.cpp"
    "test-menu-unit.cpp")

target_include_directories(${TEST_MENU_UNIT} PRIVATE
    ${PROJECT_SOURCE_DIR})

target_include_directories(${TEST_MENU_UNIT} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS})

target_compile_options(${TEST_MENU_UNIT} PRIVATE
    -L${DEP_LIBDIR}
    -std=c++11)

target_link_libraries(${TEST_MENU_UNIT} PRIVATE
    kiran-menu-backend
    ${GTKMM_LIBRARIES})


set(TEST_MENU_SIGNAL test-menu-signal)

add_executable(${TEST_MENU_SIGNAL}
    "test-menu-signal.cpp")

target_include_directories(${TEST_MENU_SIGNAL} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS})

target_compile_options(${TEST_MENU_SIGNAL} PRIVATE
    -L${DEP_LIBDIR}
    -std=c++11)

target_link_libraries(${TEST_MENU_SIGNAL} PRIVATE
    kiran-menu-backend
    ${GTKMM_LIBRARIES})


#add_test(menu_test ${TEST_MENU_UNIT})
add_test(menu_test ${TEST_MENU_SIGNAL})
