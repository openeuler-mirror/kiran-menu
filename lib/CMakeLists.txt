cmake_minimum_required(VERSION 3.0)


file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_C_FILES ./*.c)


set (ALL_FILES ${CORE_H_FILES} ${CORE_C_FILES})

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${DEP_INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PUBLIC
    -std=c99
    -L${DEP_LIBDIR})

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${DEP_LIBRARIES})

install(FILES "com.unikylin.Kiran.StartMenu.gschema.xml.in" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
    RENAME "com.unikylin.Kiran.StartMenu.gschema.xml" 
)

install(CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")
