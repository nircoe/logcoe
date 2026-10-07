cmake_minimum_required(VERSION 3.22)
project(logcoe VERSION 0.1.1 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(cmake/logcoe_config.cmake)

option(LOGCOE_BUILD_TESTS "Build the logcoe test suite" OFF)

add_library(logcoe STATIC src/logcoe.cpp)

target_include_directories(logcoe
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

logcoe_enable_strict_warnings(logcoe)

if(MINGW)
    target_link_options(logcoe INTERFACE -static-libgcc -static-libstdc++ -static)
endif()

if(LOGCOE_BUILD_TESTS)
    logcoe_fetch_testcoe()
    add_subdirectory(tests)
endif()

install(TARGETS logcoe
    EXPORT logcoe-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES include/logcoe.hpp
    DESTINATION include
)

install(EXPORT logcoe-targets
    FILE logcoe-targets.cmake
    DESTINATION lib/cmake/logcoe
)
