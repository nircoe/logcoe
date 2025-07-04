# Automatically copies required MinGW DLLs to target directory on Windows

function(copy_mingw_dlls_to_target target_name)
    if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "[logcoe] Setting up automatic MinGW DLL copying for ${target_name}")
        
        # Get the compiler directory
        get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
        
        # List of DLLs that might be needed for MinGW
        set(MINGW_DLLS
            libgcc_s_seh-1.dll     # 64-bit SEH exception handling
            libgcc_s_dw2-1.dll     # 32-bit Dwarf2 exception handling
            libstdc++-6.dll        # C++ standard library
            libwinpthread-1.dll    # Threading support
        )
        
        foreach(dll ${MINGW_DLLS})
            set(dll_path "${COMPILER_DIR}/${dll}")
            if(EXISTS "${dll_path}")
                message(STATUS "[logcoe] Found ${dll} at ${dll_path}")
                add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dll_path}"
                    "$<TARGET_FILE_DIR:${target_name}>/${dll}"
                    COMMENT "[logcoe] Copying MinGW DLL: ${dll}"
                    VERBATIM
                )
            else()
                message(STATUS "[logcoe] ${dll} not found, skipping")
            endif()
        endforeach()
    endif()
endfunction()

# Alternative function for multiple targets
function(copy_mingw_dlls_to_targets)
    foreach(target_name ${ARGN})
        copy_mingw_dlls_to_target(${target_name})
    endforeach()
endfunction()

function(ignore_external_warnings target_name)
    if(TARGET ${target_name})
        get_target_property(INCLUDE_DIRS ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
        if(INCLUDE_DIRS)
            set_target_properties(${target_name} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}"
            )
            message(STATUS "Ignoring ${target_name} warnings")
        endif()
    else()
        message(WARNING "Target ${target_name} not found")
    endif()
endfunction()
