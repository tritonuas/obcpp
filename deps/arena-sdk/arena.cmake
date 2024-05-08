function(target_add_arena target_name)
    if(UNIX AND NOT APPLE)
        # check if the ARENA_EXTRACTED_PATH environment variable is set
        # before trying to add arena to a target. 
        # See the Dockerfile for how this is set. 
        if(DEFINED ENV{ARENA_EXTRACTED_PATH})
            message("Adding ArenaSDK found at $ENV{ARENA_EXTRACTED_PATH} for target ${target_name}")
            # Add a preprocessor macro that will enable us to compile
            # functionality that depends on the Arena SDK.
            target_compile_definitions(${target_name} PRIVATE 
                ARENA_SDK_INSTALLED
            )

            target_include_directories(${target_name} PRIVATE 
                "${ARENA_SDK_DIR}/include/Arena"
                "${ARENA_SDK_DIR}/GenICam/library/CPP/include"
                "${ARENA_SDK_DIR}/include/GenTL"
                "${ARENA_SDK_DIR}/include/Save"
            )
            file(GLOB_RECURSE ARENA_LIBS 
                "${ARENA_SDK_DIR}/lib64/*.so" 
                "${ARENA_SDK_DIR}/**/*.so"
            )
            target_link_libraries(${target_name} PRIVATE
                ${ARENA_LIBS}
            )
        else()
            message("ArenaSDK not found. NOT adding to target ${target_name}. Note that the ARENA_EXTRACTED_PATH environment variable must be set to the path where ArenaSDK is extracted to.")
        endif()

    endif()
endfunction()
