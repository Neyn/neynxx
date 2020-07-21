cmake_minimum_required(VERSION 3.5)
option(NEYN_BUILD_LIB "Build Neyn C Library" OFF)
add_subdirectory(CNeyn)

project(Neynxx VERSION ${NEYN_VERSION} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NEYNXX_MASTER OFF CACHE INTERNAL "")
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(NEYNXX_MASTER ON CACHE INTERNAL "")
endif()

set(NEYNXX_VERSION ${PROJECT_VERSION} CACHE INTERNAL "")
set(NEYNXX_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")
set(NEYNXX_LIBRARIES "neynxx" CACHE INTERNAL "")
option(NEYNXX_BUILD_LIB "Build Neyn C++ Library" ON)
option(NEYNXX_BUILD_TESTS "Build Neyn C++ Tests" ${NEYNXX_MASTER})
option(NEYNXX_INSTALL_LIB "Install Neyn C++ Library" ${NEYNXX_MASTER})

file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/neynxx/*.h"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h")
file(GLOB SOURCES
    "${NEYN_INCLUDE_DIR}/neyn/*.h"
    "${NEYN_INCLUDE_DIR}/neyn/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/neynxx/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/neynxx/*.cpp")

if(NEYNXX_BUILD_LIB)
    add_library(neynxx ${SOURCES})
    target_link_libraries(neynxx PUBLIC pthread)
    target_include_directories(neynxx PUBLIC
        $<BUILD_INTERFACE:${NEYN_INCLUDE_DIR}/>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/>)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/neynxx/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
endif(NEYNXX_BUILD_LIB)

if(NEYNXX_INSTALL_LIB)
    install(TARGETS neynxx
        EXPORT neynxxConfig
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
    install(FILES ${HEADERS}
        DESTINATION include/neynxx)
    install(EXPORT neynxxConfig
        NAMESPACE neynxx::
        DESTINATION lib/cmake/neynxx)
    export(TARGETS neynxx
        NAMESPACE neynxx::
        FILE ${PROJECT_BINARY_DIR}/neynxxConfig.cmake)
endif(NEYNXX_INSTALL_LIB)

if(NEYNXX_BUILD_TESTS)
    add_executable(stage "stage/main.cpp")
    target_link_libraries(stage PUBLIC neynxx)
endif(NEYNXX_BUILD_TESTS)
