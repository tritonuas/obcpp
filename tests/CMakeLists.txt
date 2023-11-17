set(BINARY ${CMAKE_PROJECT_NAME}_tests)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)
# file(GLOB_RECURSE HEADERS "../include/*.hpp")
set(INCLUDE_DIRECTORY "../include" ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIRECTORY})

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

target_link_libraries(${BINARY} PUBLIC gtest_main opencv opencv_core)

add_test(NAME ${BINARY} COMMAND ${BINARY})

# setup make target (make test)
add_custom_target(test)
add_custom_command(
    TARGET test
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY}"
    ARGS "--output-on-failure"
)
add_dependencies(test ${BINARY})

