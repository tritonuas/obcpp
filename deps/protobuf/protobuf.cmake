function(target_add_protobuf target_name)
    include(FindProtobuf)
    
    # Find the appropriate protobuf library for the current architecture
    find_library(PROTOBUF_LIBRARY
        NAMES protobuf
        PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib/arm-linux-gnueabihf
        NO_DEFAULT_PATH
    )
    
    # Fallback to system default search
    if(NOT PROTOBUF_LIBRARY)
        find_library(PROTOBUF_LIBRARY NAMES protobuf)
    endif()
    
    if(NOT PROTOBUF_LIBRARY)
        message(FATAL_ERROR "Could not find protobuf library")
    endif()
    
    set(Protobuf_LIBRARIES ${PROTOBUF_LIBRARY})
    find_package(Protobuf REQUIRED)

    target_include_directories(${target_name} PRIVATE ${PROTOBUF_INCLUDE_DIR})

    target_link_libraries(${target_name} PRIVATE
        ${PROTOBUF_LIBRARY}
    )

endfunction()
