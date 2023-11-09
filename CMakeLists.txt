# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

# enable build caching
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)


project(obcpp VERSION 1.0)

# =============================
# Dependencies 
set(DEPS_DIRECTORY ${PROJECT_SOURCE_DIR}/deps)

include(${DEPS_DIRECTORY}/torch/torch.cmake)
include(${DEPS_DIRECTORY}/arena-sdk/arena.cmake)
include(${DEPS_DIRECTORY}/json/json.cmake)
include(${DEPS_DIRECTORY}/eigen/eigen.cmake)
include(${DEPS_DIRECTORY}/opencv/opencv.cmake)
include(${DEPS_DIRECTORY}/httplib/httplib.cmake)
# =============================

# =============================
# obcpp executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${SOURCES})
include_directories(${INCLUDE_DIRECTORY})
target_add_torch(${PROJECT_NAME})
target_add_arena(${PROJECT_NAME})
target_add_json(${PROJECT_NAME})
target_add_eigen(${PROJECT_NAME})
target_add_opencv(${PROJECT_NAME})
target_add_httplib(${PROJECT_NAME})
# =============================

# =============================
# Integration tests

# playground
add_executable(playground tests/integration/playground.cpp)
target_add_torch(playground)
target_add_arena(playground)
target_add_json(playground)
target_add_eigen(playground) 
target_add_opencv(playground) 
target_add_httplib(playground)
# =============================

# =============================
# Unit tests
add_subdirectory(tests/unit)
add_subdirectory(${DEPS_DIRECTORY}/google-test)
# =============================

# =============================
# Linting 
get_target_property(PROJECT_LIBS_INCLUDE_DIRS ${PROJECT_NAME} INCLUDE_DIRECTORIES)

foreach(dir ${PROJECT_LIBS_INCLUDE_DIRS})
    string(APPEND LIB_INCLUDE_CLANG_TIDY_STRING "-I${dir};")
endforeach()

# Adding lint target if clang-tidy executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  # define lint target
  add_custom_target(lint
    COMMAND ${CLANG_TIDY} 
    ${SOURCES}
    ${HEADERS}
    --
    -std=c++${CMAKE_CXX_STANDARD}
    -I${INCLUDE_DIRECTORY}
    # NOTE:might need to add include directory if library's 
    # include directory is not in LIB_INCLUDE_CLANG_TIDY_STRING
    -I${CMAKE_BINARY_DIR}/_deps/json-src/single_include
    ${LIB_INCLUDE_CLANG_TIDY_STRING}
   )
else()
    message(FATAL_ERROR "clang-tidy executable not found. Check the README for steps to install it on your system")
endif()
# =============================
