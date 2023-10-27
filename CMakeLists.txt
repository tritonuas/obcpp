# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

# Check if Ninja is available
find_program(NINJA_EXECUTABLE ninja)

# Set the default generator
if(NINJA_EXECUTABLE)
    set(CMAKE_GENERATOR "Ninja")
else()
    set(CMAKE_GENERATOR "Unix Makefiles") # Use a fallback generator
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

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# =============================
# Executable
set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${SOURCES})
include_directories(${INCLUDE_DIRECTORY})
# =============================

# =============================
# Libraries
set(DEPS_DIRECTORY ${PROJECT_SOURCE_DIR}/deps)

add_subdirectory(${DEPS_DIRECTORY}/arena-sdk)
add_subdirectory(${DEPS_DIRECTORY}/json)
include(${DEPS_DIRECTORY}/eigen/CMakeLists.txt)
# =============================
include(${DEPS_DIRECTORY}/torch/CMakeLists.txt)
# NOTE: temporarily leaving out opencv until we have it working in dev container
# include(${DEPS_DIRECTORY}/opencv/CMakeLists.txt)
# =============================
# Unit tests
add_subdirectory(tests)
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
