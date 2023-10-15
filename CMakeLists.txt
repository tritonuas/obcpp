set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

cmake_minimum_required(VERSION 3.10)

project(obcpp VERSION 1.0)

# =============================
# Libraries
add_library(core_library SHARED
    src/core/states.cpp
)
add_library(core::library ALIAS core_library)
target_include_directories(core_library
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
# =============================

# =============================
# Executable

add_executable(obcpp
    src/main.cpp
)

target_link_libraries(obcpp
    PRIVATE
        core::library
        nlohmann_json::nlohmann_json
)

# =============================


# =============================
# Unit tests

add_subdirectory(tests)
add_subdirectory(deps/google-test)

# enable CTest testing
enable_testing()

# Add a testing executable
add_executable(hello_test tests/hello_test.cpp)
add_executable(hello_test2 tests/hello_test2.cpp)

target_link_libraries(
    hello_test
    GTest::GTest 
    GTest::Main
)

target_link_libraries(
    hello_test2
    GTest::GTest 
    GTest::Main
)

add_test(test_all ./bin/hello_test)
add_test(test_all ./bin/hello_test2)
# =============================
# Json Library
