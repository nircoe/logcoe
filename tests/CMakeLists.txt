find_package(testcoe QUIET)
if(testcoe_FOUND)
    message(STATUS "[logcoe] Using existing testcoe installation")
else()
    message(STATUS "[logcoe] testcoe not found, fetching from source")
    include(FetchContent)
    FetchContent_Declare(
        testcoe
        GIT_REPOSITORY https://github.com/nircoe/testcoe.git
        GIT_TAG v0.1.1
    )
    FetchContent_MakeAvailable(testcoe)
endif()

ignore_external_warnings(testcoe)

add_executable(logcoe_tests main.cpp logcoe_test.cpp logcoe_thread_test.cpp)

target_link_libraries(logcoe_tests
    PRIVATE
        logcoe
        testcoe
)

add_custom_target(run_logcoe_tests
    COMMAND logcoe_tests
    DEPENDS logcoe_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(logcoe_strip_tests main.cpp logcoe_strip_test.cpp)

target_link_libraries(logcoe_strip_tests
    PRIVATE
        logcoe
        testcoe
)

add_custom_target(run_logcoe_strip_tests
    COMMAND logcoe_strip_tests
    DEPENDS logcoe_strip_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

copy_mingw_dlls_to_targets(logcoe_tests logcoe_strip_tests)