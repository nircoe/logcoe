logcoe_ignore_external_warnings(testcoe)

add_executable(logcoe_tests
    main.cpp
    logcoe_test.cpp
    logcoe_thread_test.cpp
    logcoe_error_handling_test.cpp
)

target_link_libraries(logcoe_tests
    PRIVATE
        logcoe
        testcoe
)

logcoe_enable_strict_warnings(logcoe_tests)
logcoe_enable_static_mingw_runtime(logcoe_tests)

add_custom_target(run_logcoe_tests
    COMMAND logcoe_tests
    DEPENDS logcoe_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(logcoe_strip_tests
    main.cpp
    logcoe_strip_test.cpp
)

target_link_libraries(logcoe_strip_tests
    PRIVATE
        logcoe
        testcoe
)

logcoe_enable_strict_warnings(logcoe_strip_tests)
logcoe_enable_static_mingw_runtime(logcoe_strip_tests)

add_custom_target(run_logcoe_strip_tests
    COMMAND logcoe_strip_tests
    DEPENDS logcoe_strip_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
