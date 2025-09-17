function(target_add_protobuf target_name)
    include(FindProtobuf)
    set(PROTOBUF_LIBRARY /usr/lib/x86_64-linux-gnu/libprotobuf.so CACHE FILEPATH "System protobuf lib")
    set(Protobuf_LIBRARIES ${PROTOBUF_LIBRARY})
    find_package(Protobuf REQUIRED)

    target_include_directories(${target_name} PRIVATE ${PROTOBUF_INCLUDE_DIR})

    target_link_libraries(${target_name} PRIVATE
        ${PROTOBUF_LIBRARY}
    )

endfunction()
