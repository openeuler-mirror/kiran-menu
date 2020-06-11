cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

pkg_search_module(GTKMM REQUIRED gtkmm-3.0)

set(DEP_LIBDIR ${GTKMM_LIBDIR})

set(TEST_MENU_UNIT test-menu-unit)
set(TEST_MENU_SIGNAL test-menu-signal)
set(TEST_DISPLAY test-display)

add_executable(${TEST_MENU_UNIT}
    "test-all-apps.cpp"
    "test-app-category.cpp"
    "test-app-favorite.cpp"
    "test-app-frequent.cpp"
    "test-app-search.cpp"
    "test-menu-unit.cpp")

add_executable(${TEST_MENU_SIGNAL}
    "test-menu-signal.cpp")

add_executable(${TEST_DISPLAY}
    "test-display.cpp")

list (APPEND TESTS ${TEST_MENU_UNIT} ${TEST_MENU_SIGNAL} ${TEST_DISPLAY})

foreach(TEST_ITEM IN ITEMS ${TESTS})

    target_include_directories(${TEST_ITEM} PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${GTKMM_INCLUDE_DIRS})

    target_compile_options(${TEST_ITEM} PRIVATE
        -L${DEP_LIBDIR}
        -std=c++11)

    target_link_libraries(${TEST_ITEM} PRIVATE
        kiran-menu-backend
        ${GTKMM_LIBRARIES})

    add_test(menu_test ${TEST_ITEM})
endforeach()


