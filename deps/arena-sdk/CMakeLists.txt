if(UNIX AND NOT APPLE)
    set(ARENA_SDK_DIR "${CMAKE_BINARY_DIR}/_deps/arena-sdk")
    execute_process(
        COMMAND "${PROJECT_SOURCE_DIR}/deps/arena-sdk/install-arena.sh" ${ARENA_SDK_DIR} 
    )
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
    # Add a preprocessor macro that will enable us to compile
    # functionality that depends on the Arena SDK.
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        ARENA_SDK_INSTALLED
    )
endif()
