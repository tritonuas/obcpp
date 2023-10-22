if(UNIX AND NOT APPLE)
    set(ARENA_SDK_DIR "${CMAKE_BINARY_DIR}/_deps/arena-sdk")
    set(INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/deps/arena-sdk/install-arena.sh")

    # run install script with output directory as argument
    execute_process(
        COMMAND sh ${INSTALL_SCRIPT} ${ARENA_SDK_DIR} 
        RESULT_VARIABLE ret
    )
    if(ret EQUAL "0")
        # Add a preprocessor macro that will enable us to compile
        # functionality that depends on the Arena SDK.
        target_compile_definitions(${PROJECT_NAME} PRIVATE 
            ARENA_SDK_INSTALLED
        )
    else()
        message(FATAL_ERROR "Unable to install Arena-SDK. ${INSTALL_SCRIPT} script exited with code ${ret}.")
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE 
        "${ARENA_SDK_DIR}/extracted/include/Arena"
        "${ARENA_SDK_DIR}/extracted/GenICam/library/CPP/include"
    )
    file(GLOB_RECURSE ARENA_LIBS 
        "${ARENA_SDK_DIR}/extracted/lib64/*.so" 
        "${ARENA_SDK_DIR}/extracted/ffmpeg/*.so"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${ARENA_LIBS}
    )
endif()
