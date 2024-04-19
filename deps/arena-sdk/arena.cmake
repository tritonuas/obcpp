function(target_add_arena target_name)
    if(UNIX AND NOT APPLE)
        set(ARENA_SDK_DIR "/arena-tmp/ArenaSDK_Linux_ARM64")
        # set(INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/deps/arena-sdk/install-arena.sh")

        # run install script with output directory as argument
        # execute_process(
        #     COMMAND sh ${INSTALL_SCRIPT} ${ARENA_SDK_DIR} 
        #     RESULT_VARIABLE ret
        # )
        # if(ret EQUAL "0")
            # Add a preprocessor macro that will enable us to compile
            # functionality that depends on the Arena SDK.
            target_compile_definitions(${target_name} PRIVATE 
                ARENA_SDK_INSTALLED
            )
        # else()
        #     message(FATAL_ERROR "Unable to install Arena-SDK. ${INSTALL_SCRIPT} script exited with code ${ret}.")
        # endif()

        target_include_directories(${target_name} PRIVATE 
            "${ARENA_SDK_DIR}/include/Arena"
            "${ARENA_SDK_DIR}/GenICam/library/CPP/include"
            "${ARENA_SDK_DIR}/include/GenTL"
            "${ARENA_SDK_DIR}/include/Save"
        )
        # file(GLOB_RECURSE ARENA_LIBS 
        #     # "${ARENA_SDK_DIR}/extracted/lib64/*.so" 
        #     "${ARENA_SDK_DIR}/extracted/**/*.so"
        # )
        # target_link_libraries(${target_name} PRIVATE
        #     ${ARENA_LIBS}
        # )
    endif()
endfunction()
