find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "Using existing GoogleTest installation")
else()
    message(STATUS "GoogleTest not found, fetching from source")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

enable_testing()

add_executable(logcoe_tests logcoe_test.cpp logcoe_thread_test.cpp)

if(GTest_FOUND)
    target_link_libraries(logcoe_tests
        PRIVATE
            logcoe
            GTest::GTest
            GTest::Main
    )
else()
    target_link_libraries(logcoe_tests
        PRIVATE
            logcoe
            gtest
            gtest_main 
    )
endif()

add_test(NAME logcoe_tests COMMAND logcoe_tests)