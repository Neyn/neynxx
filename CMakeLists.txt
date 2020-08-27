cmake_minimum_required(VERSION 3.5)
option(CNEYN_BUILD_LIB "Build Neyn C Library" OFF)
add_subdirectory(cneyn)

project(Neyn VERSION ${CNEYN_VERSION} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NEYN_MASTER OFF CACHE INTERNAL "")
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(NEYN_MASTER ON CACHE INTERNAL "")
endif()

set(NEYN_VERSION ${PROJECT_VERSION} CACHE INTERNAL "")
set(NEYN_LIBRARIES "neyn" CACHE INTERNAL "")
set(NEYN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/" CACHE INTERNAL "")

option(NEYN_BUILD_LIB "Build Neyn C++ Library" ON)
option(NEYN_BUILD_TESTS "Build Neyn C++ Tests" ${NEYN_MASTER})
option(NEYN_INSTALL_LIB "Install Neyn C++ Library" ${NEYN_MASTER})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/neyn/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/neyn/config.h")
file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/neyn/neyn.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/neyn/server.h"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/neyn/config.h")
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/neyn/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/neyn/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cneyn/src/cneyn/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/cneyn/src/cneyn/*.c")

if(NEYN_BUILD_LIB)
    add_library(neyn ${SOURCES})
    target_link_libraries(neyn PUBLIC pthread)
    target_include_directories(neyn PUBLIC
        "$<BUILD_INTERFACE:${CNEYN_INCLUDE_DIR}/>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/gen/>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/>")
endif()

if(NEYN_INSTALL_LIB)
    install(TARGETS neyn
        EXPORT neynConfig
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
    install(FILES ${HEADERS}
        DESTINATION include/neyn)
    install(EXPORT neynConfig
        NAMESPACE neyn::
        DESTINATION lib/cmake/neyn)
    export(TARGETS neyn
        NAMESPACE neyn::
        FILE "${PROJECT_BINARY_DIR}/neynConfig.cmake")
endif()

if(NEYN_BUILD_TESTS)
    add_executable(stage "test/stage.cpp")
    target_link_libraries(stage PUBLIC neyn)

    add_executable(tests "test/tests.cpp")
    target_link_libraries(tests PUBLIC neyn)
endif()
