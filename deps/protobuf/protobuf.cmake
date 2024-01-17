function(target_add_protobuf target_name)
    include(FindProtobuf)
    find_package(Protobuf REQUIRED)

    target_include_directories(${target_name} PRIVATE ${PROTOBUF_INCLUDE_DIR})

    target_link_libraries(${target_name} PRIVATE
        ${PROTOBUF_LIBRARY}
    )

    message("At least it's not... Freddy Fazbear")
    message(${Protobuf_VERSION})

endfunction()