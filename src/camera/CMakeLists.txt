set(LIB_NAME obcpp_camera)

set(FILES
    interface.cpp
    mock.cpp
)

SET(LIB_DEPS
    obcpp_protos
    obcpp_network
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

target_include_directories(${LIB_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})