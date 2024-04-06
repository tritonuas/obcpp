set(BINARY ${CMAKE_PROJECT_NAME}_tests)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.h *.cpp *.c)

# remove the implementation of sockets that actually use syscalls, to be replaced
# by tests/unit/airdrop_client/mock_airdrop_sockets.c which will be globbed in TEST_SOURCES
set(SOURCES_CULLED ${SOURCES})
list(REMOVE_ITEM SOURCES_CULLED ${PROJECT_SOURCE_DIR}/src/network/airdrop_sockets.c)

add_executable(${BINARY} ${TEST_SOURCES} ${SOURCES_CULLED})

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(GEN_PROTOS_DIRECTORY ${PROJECT_SOURCE_DIR}/build/gen_protos)
include_directories(${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY} ${PROJECT_SOURCE_DIR}/tests/unit/airdrop_client)

target_link_libraries(${BINARY} PUBLIC gtest_main)
target_add_protobuf(${BINARY})
target_add_matplot(${BINARY})
target_add_opencv(${BINARY}) 
target_add_torch(${BINARY}) 
target_add_httplib(${BINARY})
target_add_json(${BINARY})
target_add_mavsdk(${BINARY})
target_add_matplot(${BINARY})
target_add_httplib(${BINARY})
target_add_loguru(${BINARY})
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

