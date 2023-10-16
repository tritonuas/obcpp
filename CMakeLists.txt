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

add_subdirectory(deps/json)
add_subdirectory(deps/cpp-httplib)
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
        httplib
)

# =============================


# =============================
# Unit tests

add_subdirectory(tests)
add_subdirectory(deps/google-test)

# =============================
# Json Library
