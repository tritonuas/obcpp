set(LIB_NAME obcpp_ticks)

set(FILES
    airdrop_approach.cpp
    auto_landing.cpp
    cv_loiter.cpp
    fly_search.cpp
    fly_waypoints.cpp
    manual_landing.cpp
    mission_done.cpp
    mission_prep.cpp
    mission_upload.cpp
    path_gen.cpp
    path_validate.cpp
    takeoff.cpp
)

set(LIB_DEPS
    obcpp_pathing
    obcpp_utilities
    obcpp_core
    obcpp_network
)

add_library(${LIB_NAME} STATIC ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc
    ${FILES}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${LIB_DEPS}
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