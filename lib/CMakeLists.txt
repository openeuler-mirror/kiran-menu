cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME kiran-menu-backend)

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)

set(ALL_FILES ${CORE_H_FILES} ${CORE_CPP_FILES})

add_library(${TARGET_NAME} ${ALL_FILES})

set(DEP_LIBDIR ${GLIBMM_LIBDIR})

include_directories(${CMAKE_BINARY_DIR})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}
         ${GTHREAD_INCLUDE_DIRS}
         ${WNCK3_INCLUDE_DIRS}
         ${GDKMM3_INCLUDE_DIRS}
         ${X11_XCB_INCLUDE_DIRS}
         ${XCB_RES_INCLUDE_DIRS}
         ${XCOMPOSITE_INCLUDE_DIRS}
         ${KLOG_GTK3_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PUBLIC -DWNCK_I_KNOW_THIS_IS_UNSTABLE
                                             -Wno-parentheses)

target_compile_options(${TARGET_NAME} PRIVATE -L${DEP_LIBDIR}
                                              -DG_LOG_DOMAIN=\"${TARGET_NAME}\")

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${GTHREAD_LIBRARIES}
          ${WNCK3_LIBRARIES}
          ${GDKMM3_LIBRARIES}
          ${X11_XCB_LIBRARIES}
          ${XCB_RES_LIBRARIES}
          ${XCOMPOSITE_LIBRARIES}
          ${KLOG_GTK3_LIBRARIES})
