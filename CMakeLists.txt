cmake_minimum_required(VERSION 3.14)
project(logcoe VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(logcoe STATIC src/logcoe.cpp)

target_include_directories(logcoe
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(cmake/utils.cmake)

option(LOGCOE_BUILD_TESTS "Build the logcoe test suite" OFF)
if(LOGCOE_BUILD_TESTS)
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


