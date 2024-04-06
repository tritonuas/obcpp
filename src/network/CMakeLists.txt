set(LIB_NAME obcpp_network)

add_library(${LIB_NAME} STATIC ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc
    airdrop_client.cpp
    airdrop_sockets.c
    gcs_routes.cpp
    gcs.cpp
    mavlink.cpp
)

target_link_libraries(${LIB_NAME} PRIVATE
    obcpp_utilities
    obcpp_ticks
    obcpp_core
    obcpp_pathing
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

set(LIB_NAME obcpp_network_mock)

add_library(${LIB_NAME} STATIC ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc
    airdrop_client.cpp
    mock/airdrop_sockets.c
    mock/packet_queue.c
    gcs_routes.cpp
    gcs.cpp
    mavlink.cpp
)

target_link_libraries(${LIB_NAME} PRIVATE
    obcpp_utilities
    obcpp_ticks
    obcpp_core_mock
    obcpp_pathing
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