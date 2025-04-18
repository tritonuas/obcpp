add_library(obcpp_protos STATIC 
    ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc 
)

add_library(obcpp_udp2 STATIC
    ${PROJECT_SOURCE_DIR}/include/udp_squared/internal/helper.c
)

add_subdirectory(camera)
add_subdirectory(core)
add_subdirectory(cv)
add_subdirectory(network)
add_subdirectory(pathing)
add_subdirectory(ticks)
add_subdirectory(utilities)

add_library(obcpp_lib INTERFACE)
target_link_libraries(obcpp_lib INTERFACE
    obcpp_protos
    obcpp_camera
    obcpp_core
    obcpp_cv
    obcpp_network
    obcpp_pathing
    obcpp_ticks
    obcpp_utilities
    obcpp_udp2
)

# for use in unit tests
add_library(obcpp_lib_mock INTERFACE)
target_link_libraries(obcpp_lib_mock INTERFACE
    obcpp_protos
    obcpp_camera
    obcpp_core
    obcpp_cv
    obcpp_network_mock
    obcpp_pathing
    obcpp_ticks
    obcpp_utilities
    obcpp_udp2
)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    obcpp_lib
    Boost::serialization
)
target_add_protobuf(${PROJECT_NAME})
target_add_torch(${PROJECT_NAME})
target_add_torchvision(${PROJECT_NAME})
target_add_matplot(${PROJECT_NAME})
target_add_json(${PROJECT_NAME})
target_add_mavsdk(${PROJECT_NAME})
target_add_opencv(${PROJECT_NAME})
target_add_httplib(${PROJECT_NAME})
target_add_loguru(${PROJECT_NAME})
target_add_boost(${PROJECT_NAME})
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
