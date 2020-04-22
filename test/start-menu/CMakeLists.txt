cmake_minimum_required(VERSION 2.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(TARGET_NAME test-start-menu)

file(GLOB_RECURSE MENU_TEST_H_FILES ./*.h)
file(GLOB_RECURSE MENU_TEST_C_FILES ./*.c)

set (ALL_FILES ${MENU_TEST_H_FILES} ${MENU_TEST_C_FILES})


add_executable(${TARGET_NAME} ${ALL_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR})

target_compile_options(${TARGET_NAME} PRIVATE
    -std=c99
    -L${DEP_LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    kiran-menu-backend
    ${DEP_LIBRARIES})


add_test(menu_test ${TARGET_NAME})
