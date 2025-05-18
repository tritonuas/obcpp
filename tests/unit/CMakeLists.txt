set(BINARY ${CMAKE_PROJECT_NAME}_tests)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.h *.cpp *.c)

add_executable(${BINARY} ${TEST_SOURCES})# ${PROJECT_SOURCE_DIR}/build/gen_protos/protos/obc.pb.cc)
target_link_libraries(${BINARY} PRIVATE obcpp_lib_mock obcpp_cv) 

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(GEN_PROTOS_DIRECTORY ${PROJECT_SOURCE_DIR}/build/gen_protos)
target_include_directories(${BINARY} PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})

target_link_libraries(${BINARY} PUBLIC gtest_main)
target_add_protobuf(${BINARY})
target_add_torch(${BINARY}) 
target_add_torchvision(${PROJECT_NAME})
target_add_matplot(${BINARY})
target_add_httplib(${BINARY})
target_add_json(${BINARY})
target_add_mavsdk(${BINARY})
target_add_opencv(${BINARY}) 
target_add_matplot(${BINARY})
target_add_httplib(${BINARY})
target_add_loguru(${BINARY})
target_add_onnxruntime(${BINARY})
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(${BINARY} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${BINARY} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

# setup make target (make test)
add_custom_target(test)
add_custom_command(
    TARGET test
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY}"
    ARGS "--output-on-failure"
)
add_dependencies(test ${BINARY})

