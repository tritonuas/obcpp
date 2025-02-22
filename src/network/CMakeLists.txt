set(LIB_NAME obcpp_network)

set(COMMON_FILES
    airdrop_client.cpp
    gcs_routes.cpp
    gcs.cpp
    mavlink.cpp
)

set(FILES
    airdrop_sockets.c
    ${COMMON_FILES}
)

set(MOCK_FILES
    mock/airdrop_sockets.c
    mock/packet_queue.c
    ${COMMON_FILES}
)

set(LIB_DEPS
    obcpp_protos
    obcpp_utilities
    obcpp_ticks
    obcpp_core
    obcpp_pathing
)

add_library(${LIB_NAME} STATIC
    ${FILES}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${LIB_DEPS}
)

set_unity_for_target(${LIB_NAME})

target_add_protobuf(${LIB_NAME})
target_add_torch(${LIB_NAME})
target_add_torchvision(${LIB_NAME})
target_add_json(${LIB_NAME})
target_add_opencv(${LIB_NAME})
target_add_httplib(${LIB_NAME})
target_add_mavsdk(${LIB_NAME})
target_add_matplot(${LIB_NAME})
target_add_loguru(${LIB_NAME})
target_add_onnxruntime(${LIB_NAME})

target_include_directories(${LIB_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

set(LIB_NAME obcpp_network_mock)

add_library(${LIB_NAME} STATIC ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc
    ${MOCK_FILES}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${LIB_DEPS}
)

set_unity_for_target(${LIB_NAME})

target_add_protobuf(${LIB_NAME})
target_add_torch(${LIB_NAME})
target_add_torchvision(${LIB_NAME})
target_add_json(${LIB_NAME})
target_add_opencv(${LIB_NAME})
target_add_httplib(${LIB_NAME})
target_add_mavsdk(${LIB_NAME})
target_add_matplot(${LIB_NAME})
target_add_loguru(${LIB_NAME})

target_include_directories(${LIB_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})