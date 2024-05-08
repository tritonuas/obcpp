function(target_add_arena target_name)
    if(UNIX AND NOT APPLE)
        # check if the ARENA_EXTRACTED_PATH environment variable is set
        # before trying to add arena to a target. 
        # See the Dockerfile for how this is set. 
        set(ARENA_SDK_DIR $ENV{ARENA_EXTRACTED_PATH})
        if(NOT "${ARENA_SDK_DIR}" STREQUAL "")
            message("Adding ArenaSDK for target ${target_name}")
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
        endif()

    endif()
endfunction()
