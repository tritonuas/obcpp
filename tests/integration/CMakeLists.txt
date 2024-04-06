# playground
add_executable(playground "playground.cpp")
target_link_libraries(playground PRIVATE obcpp_lib)
target_include_directories(playground PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
target_add_torch(playground)
# target_add_arena(playground)
target_add_json(playground)
target_add_opencv(playground) 
target_add_httplib(playground)
target_add_mavsdk(playground)
target_add_matplot(playground)
target_add_protobuf(playground)
target_add_loguru(playground)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(playground PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(playground PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# load_torchvision_model
add_executable(load_torchvision_model  "load_torchvision_model.cpp")
target_link_libraries(load_torchvision_model PRIVATE obcpp_lib)
target_add_torch(load_torchvision_model)
target_add_torchvision(load_torchvision_model)
target_add_loguru(load_torchvision_model)

# mavlink-client
add_executable(mavlink_client "mavlink_client.cpp")
target_link_libraries(mavlink_client PRIVATE obcpp_lib)
target_include_directories(mavlink_client PRIVATE ${INCLUDE_DIRECTORY})
target_add_protobuf(mavlink_client)
target_add_torch(mavlink_client)
target_add_torchvision(mavlink_client)
target_add_json(mavlink_client)
target_add_opencv(mavlink_client)
target_add_httplib(mavlink_client)
target_add_mavsdk(mavlink_client)
target_add_matplot(mavlink_client)
target_add_loguru(mavlink_client)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(mavlink_client PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(mavlink_client PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# cv_pipeline 
add_executable(cv_pipeline "cv_pipeline.cpp")
target_link_libraries(cv_pipeline PRIVATE obcpp_lib)
target_add_json(cv_pipeline)
target_add_matplot(cv_pipeline)
target_add_opencv(cv_pipeline)
target_add_loguru(cv_pipeline)
target_add_httplib(cv_pipeline)
target_add_protobuf(cv_pipeline)
target_add_torch(cv_pipeline)
target_add_torchvision(cv_pipeline)
target_add_mavsdk(cv_pipeline)
target_add_loguru(cv_pipeline)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_pipeline PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_pipeline PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# cv_aggregator 
add_executable(cv_aggregator ${SOURCES} "cv_aggregator.cpp")
target_link_libraries(cv_aggregator PRIVATE obcpp_lib)
target_add_json(cv_aggregator)
target_add_matplot(cv_aggregator)
target_add_opencv(cv_aggregator)
target_add_loguru(cv_aggregator)
target_add_httplib(cv_aggregator)
target_add_protobuf(cv_aggregator)
target_add_torch(cv_aggregator)
target_add_torchvision(cv_aggregator)
target_add_mavsdk(cv_aggregator)
target_add_loguru(cv_aggregator)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_aggregator PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_aggregator PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# cv_matching
add_executable(cv_matching "cv_matching.cpp")
target_link_libraries(cv_matching PRIVATE obcpp_lib)
target_add_json(cv_matching)
target_add_matplot(cv_matching)
target_add_opencv(cv_matching)
target_add_torch(cv_matching)
target_add_torchvision(cv_matching)
target_add_loguru(cv_matching)
target_add_httplib(cv_matching)
target_add_mavsdk(cv_matching)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_matching PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_matching PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})


# cv_segmentation
add_executable(cv_segmentation "cv_segmentation.cpp")
target_link_libraries(cv_segmentation PRIVATE obcpp_lib)
target_add_json(cv_segmentation)
target_add_matplot(cv_segmentation)
target_add_opencv(cv_segmentation)
target_add_torch(cv_segmentation)
target_add_torchvision(cv_segmentation)
target_add_loguru(cv_segmentation)
target_add_httplib(cv_segmentation)
target_add_mavsdk(cv_segmentation)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_segmentation PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_segmentation PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
# path_plotting 
add_executable(path_plotting "path_plotting.cpp")
target_link_libraries(path_plotting PRIVATE obcpp_lib)
target_add_torch(path_plotting)
target_add_json(path_plotting)
target_add_httplib(path_plotting)
target_add_mavsdk(path_plotting)
target_add_matplot(path_plotting)
target_add_protobuf(path_plotting)
target_add_opencv(path_plotting) 
target_add_loguru(path_plotting)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(path_plotting PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(path_plotting PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

add_executable(path_planning "path_planning.cpp")
target_link_libraries(path_planning PRIVATE obcpp_lib)
target_include_directories(path_planning PRIVATE ${INCLUDE_DIRECTORY})
target_add_protobuf(path_planning)
target_add_torch(path_planning)
target_add_torchvision(path_planning)
target_add_json(path_planning)
target_add_opencv(path_planning)
target_add_httplib(path_planning)
target_add_mavsdk(path_planning)
target_add_matplot(path_planning)
target_add_loguru(path_planning)

target_include_directories(path_planning PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(path_planning PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# airdrop_sockets
# add_executable(airdrop_sockets src/network/airdrop_sockets.c tests/integration/airdrop_sockets.c)
# target_include_directories(airdrop_sockets PRIVATE ${INCLUDE_DIRECTORY})

# add_executable(camera_playground camera_playground.cpp)
# target_include_directories(camera_playground PRIVATE ${INCLUDE_DIRECTORY})
# target_add_arena(camera_playground)
# add_custom_target(
#     run_camera_playground
#     ${CMAKE_COMMAND}
#     -E env LD_LIBRARY_s/target_siamese_1.pt
# )