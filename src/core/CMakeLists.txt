set(LIB_NAME obcpp_core)

set(FILES
    mission_parameters.cpp
    mission_state.cpp
    obc.cpp
)

set(LIB_DEPS
    obcpp_camera
    obcpp_cv
    obcpp_network
    obcpp_pathing
    obcpp_protos
    obcpp_ticks
    obcpp_utilities
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