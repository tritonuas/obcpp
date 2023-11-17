set(BINARY ${CMAKE_PROJECT_NAME}_tests)
set(DEPS_DIRECTORY ${PROJECT_SOURCE_DIR}/deps)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.cpp)

add_executable(${BINARY} ${TEST_SOURCES} ${SOURCES})

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRECTORY})

target_link_libraries(${BINARY} PUBLIC gtest_main)
include(${DEPS_DIRECTORY}/eigen/eigen.cmake)
target_add_eigen(${BINARY})
target_add_matplot(${BINARY})
target_add_opencv(${BINARY}) 
target_add_httplib(${BINARY})
target_add_json(${BINARY})
target_add_mavsdk(${BINARY})
target_add_matplot(${BINARY})

add_test(NAME ${BINARY} COMMAND ${BINARY})

# setup make target (make test)
add_custom_target(test)
add_custom_command(
    TARGET test
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY}"
    ARGS "--output-on-failure"
)
add_dependencies(test ${BINARY})

