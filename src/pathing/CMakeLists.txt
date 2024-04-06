set(FILES
    cartesian.cpp
    dubins.cpp
    environment.cpp
    plotting.cpp
    static.cpp
    tree.cpp
)

function(create_pathing_lib mock files)
    if (mock)
        set(LIB_NAME obcpp_pathing)
    else()
        set(LIB_NAME obcpp_pathing_mock)
    endif()

    add_library(${LIB_NAME} STATIC ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc
        ${files}
    )

    target_link_libraries(${LIB_NAME} PRIVATE 
        obcpp_core
        obcpp_network
        obcpp_utilities
    )

    target_add_protobuf(${LIB_NAME})
    target_add_torch(${LIB_NAME})
    # target_add_arena(${LIB_NAME}) # Tyler: currently broken, so we had to comment this out
    target_add_torchvision(${LIB_NAME})
    target_add_json(${LIB_NAME})
    target_add_opencv(${LIB_NAME})
    target_add_httplib(${LIB_NAME})
    target_add_mavsdk(${LIB_NAME})
    target_add_matplot(${LIB_NAME})
    target_add_loguru(${LIB_NAME})
    target_include_directories(${LIB_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
endfunction()

create_pathing_lib(FALSE, FILES)
create_pathing_lib(TRUE, FILES)