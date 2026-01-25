# LIST OF ALL TARGETS
# add_executable(EXECUTABLE_NAME "SRC_FILE_WITH_MAIN.cpp")
# target_link_libraries(EXECUTABLE_NAME PRIVATE obcpp_lib)
# target_include_directories(EXECUTABLE_NAME PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
# target_add_torch(EXECUTABLE_NAME)
# target_add_json(EXECUTABLE_NAME)
# target_add_opencv(EXECUTABLE_NAME) 
# target_add_httplib(EXECUTABLE_NAME)
# target_add_mavsdk(EXECUTABLE_NAME)
# target_add_matplot(EXECUTABLE_NAME)
# target_add_protobuf(EXECUTABLE_NAME)
# target_add_loguru(EXECUTABLE_NAME)
# target_add_onnxruntime(EXECUTABLE_NAME)
# # for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# # target_add_imagemagick(path_plotting)
# target_include_directories(EXECUTABLE_NAME PRIVATE ${ImageMagick_INCLUDE_DIRS})
# target_link_libraries(EXECUTABLE_NAME PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# playground
add_executable(playground "playground.cpp")
target_link_libraries(playground PRIVATE obcpp_lib)
target_include_directories(playground PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
target_add_torch(playground)
target_add_json(playground)
target_add_opencv(playground) 
target_add_httplib(playground)
target_add_mavsdk(playground)
target_add_matplot(playground)
target_add_protobuf(playground)
target_add_loguru(playground)
target_add_onnxruntime(playground)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(playground PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(playground PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# mavlink-client
add_executable(mavlink_client "mavlink_client.cpp")
target_link_libraries(mavlink_client PRIVATE obcpp_lib)
target_include_directories(mavlink_client PRIVATE ${INCLUDE_DIRECTORY})
target_add_mavsdk(mavlink_client)
target_add_matplot(mavlink_client)
target_add_loguru(mavlink_client)

# cv_pipeline 
add_executable(cv_pipeline "cv_pipeline.cpp")
target_link_libraries(cv_pipeline PRIVATE obcpp_lib)
target_add_json(cv_pipeline)
target_add_matplot(cv_pipeline)
target_add_loguru(cv_pipeline)
target_add_mavsdk(cv_pipeline)

# cv_aggregator 
add_executable(cv_aggregator ${SOURCES} "cv_aggregator.cpp")
target_link_libraries(cv_aggregator PRIVATE obcpp_lib)
target_add_json(cv_aggregator)
target_add_matplot(cv_aggregator)
target_add_loguru(cv_aggregator)
target_add_mavsdk(cv_aggregator)

# cv_preprocess
add_executable(cv_preprocess ${SOURCES} "cv_preprocess.cpp")
target_link_libraries(cv_preprocess PRIVATE obcpp_lib)
target_include_directories(cv_preprocess PRIVATE ${INCLUDE_DIRECTORY})
target_add_matplot(cv_preprocess)

# cv_mapping
add_executable(cv_mapping ${SOURCES} "cv_mapping.cpp")
target_link_libraries(cv_mapping PRIVATE obcpp_lib)
target_include_directories(cv_mapping PRIVATE ${INCLUDE_DIRECTORY})

# cv_yolo
add_executable(cv_yolo ${SOURCES} "cv_yolo.cpp")
target_link_libraries(cv_yolo PRIVATE obcpp_lib)
target_include_directories(cv_yolo PRIVATE ${INCLUDE_DIRECTORY})

# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# path_plotting 
add_executable(path_plotting "path_plotting.cpp")
target_link_libraries(path_plotting PRIVATE obcpp_lib)
target_include_directories(path_plotting PRIVATE ${INCLUDE_DIRECTORY})
target_add_matplot(path_plotting)

add_executable(path_planning "path_planning.cpp")
target_link_libraries(path_planning PRIVATE obcpp_lib)
target_include_directories(path_planning PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(path_planning)
target_add_httplib(path_planning)
target_add_mavsdk(path_planning)
target_add_matplot(path_planning)
target_add_loguru(path_planning)

add_executable(coverage_pathing "coverage_pathing.cpp")
target_link_libraries(coverage_pathing PRIVATE obcpp_lib)
target_include_directories(coverage_pathing PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(coverage_pathing)
target_add_httplib(coverage_pathing)
target_add_mavsdk(coverage_pathing)
target_add_matplot(coverage_pathing)
target_add_loguru(coverage_pathing)


add_executable(airdrop_approach "airdrop_approach.cpp")
target_link_libraries(airdrop_approach PRIVATE obcpp_lib)
target_include_directories(airdrop_approach PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(airdrop_approach)
target_add_httplib(airdrop_approach)
target_add_mavsdk(airdrop_approach)
target_add_matplot(airdrop_approach)
target_add_loguru(airdrop_approach)

add_executable(deviation_ranking "deviation_ranking.cpp")
target_link_libraries(deviation_ranking PRIVATE obcpp_lib)
target_include_directories(deviation_ranking PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(deviation_ranking)
target_add_httplib(deviation_ranking)
target_add_mavsdk(deviation_ranking)
target_add_matplot(deviation_ranking)
target_add_loguru(deviation_ranking)


add_executable(airdrop_packets "airdrop_packets.cpp")
target_link_libraries(airdrop_packets PRIVATE obcpp_lib)
target_include_directories(airdrop_packets PRIVATE ${INCLUDE_DIRECTORY})
target_add_protobuf(airdrop_packets)
target_add_matplot(airdrop_packets)
target_add_loguru(airdrop_packets)

# airdrop_sockets
# add_executable(airdrop_sockets src/network/airdrop_sockets.c tests/integration/airdrop_sockets.c)
# target_include_directories(airdrop_sockets PRIVATE ${INCLUDE_DIRECTORY})

# add_executable(camera_playground camera_playground.cpp)
# target_include_directories(camera_playground PRIVATE ${INCLUDE_DIRECTORY})
# add_custom_target(
#     run_camera_playground
#     ${CMAKE_COMMAND}
#     -E env LD_LIBRARY_s/target_siamese_1.pt
# )



add_executable(udp_client_mock "udp_client_mock.cpp")
target_link_libraries(udp_client_mock PRIVATE 
    obcpp_lib
)
target_include_directories(udp_client_mock PRIVATE ${INCLUDE_DIRECTORY})
target_add_boost(udp_client_mock)
target_add_protobuf(udp_client_mock)
target_add_torch(udp_client_mock)
target_add_torchvision(udp_client_mock)
target_add_json(udp_client_mock)
target_add_opencv(udp_client_mock)
target_add_httplib(udp_client_mock)
target_add_mavsdk(udp_client_mock)
target_add_matplot(udp_client_mock)
target_add_loguru(udp_client_mock)
target_include_directories(udp_client_mock PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(udp_client_mock PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})



add_executable(udp_server_mock "udp_server_mock.cpp")
target_link_libraries(udp_server_mock PRIVATE 
    obcpp_lib
)
target_include_directories(udp_server_mock PRIVATE ${INCLUDE_DIRECTORY})
target_add_boost(udp_server_mock)
target_add_protobuf(udp_server_mock)
target_add_torch(udp_server_mock)
target_add_torchvision(udp_server_mock)
target_add_json(udp_server_mock)
target_add_opencv(udp_server_mock)
target_add_httplib(udp_server_mock)
target_add_mavsdk(udp_server_mock)
target_add_matplot(udp_server_mock)
target_add_loguru(udp_server_mock)
target_include_directories(udp_server_mock PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(udp_server_mock PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

add_executable(drop_it_like_its_hot "drop_it_like_its_hot.cpp")
target_link_libraries(drop_it_like_its_hot PRIVATE obcpp_lib)
target_include_directories(drop_it_like_its_hot PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(drop_it_like_its_hot)
target_add_mavsdk(drop_it_like_its_hot)
target_add_matplot(drop_it_like_its_hot)
target_add_loguru(drop_it_like_its_hot)

add_executable(picamera "picamera.cpp")
target_link_libraries(picamera PRIVATE obcpp_lib obcpp_camera obcpp_network)
target_include_directories(picamera PRIVATE ${INCLUDE_DIRECTORY})
target_add_json(picamera)
target_add_mavsdk(picamera)
target_add_matplot(picamera)
target_add_loguru(picamera)

add_executable(clustering "clustering.cpp")
target_link_libraries(clustering PRIVATE obcpp_lib obcpp_camera)
target_add_matplot(clustering) 
target_add_json(clustering)
target_add_protobuf(clustering)

add_executable(camera_integration_test "camera_integration_test.cpp")
target_link_libraries(camera_integration_test PRIVATE obcpp_lib)
target_include_directories(camera_integration_test PRIVATE ${INCLUDE_DIRECTORY})
target_add_loguru(camera_integration_test)
target_add_opencv(camera_integration_test)
target_add_boost(camera_integration_test)
target_add_mavsdk(camera_integration_test)
target_add_matplot(camera_integration_test)
target_add_json(camera_integration_test)
target_include_directories(camera_integration_test PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(camera_integration_test PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
