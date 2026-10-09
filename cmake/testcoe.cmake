function(logcoe_fetch_testcoe)
    if(TARGET testcoe)
        message(STATUS "[logcoe] testcoe already available, skipping fetch")
        return()
    endif()

    message(STATUS "[logcoe] Fetching testcoe from source...")

    FetchContent_Declare(
        testcoe
        GIT_REPOSITORY https://github.com/nircoe/testcoe.git
        GIT_TAG v0.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(testcoe)
endfunction()
