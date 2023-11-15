cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
       If you think this error is wrong, try clearing cache:
       `find -name CMakeCache.txt -delete`
")
endif()

# enable build caching
find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif(CCACHE)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

SET(GCC_COMPILE_FLAGS "")
SET(GCC_LINK_FLAGS    "-lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_flann -lopencv_imgcodecs -lopencv_dnn -lopencv_videoio -lopencv_imgproc -lopencv_ml -lopencv_photo")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

project(obcpp VERSION 1.0)

# =============================
# Dependencies 
set(FETCHCONTENT_QUIET FALSE)
set(DEPS_DIRECTORY ${PROJECT_SOURCE_DIR}/deps)

include(${DEPS_DIRECTORY}/torch/torch.cmake)
include(${DEPS_DIRECTORY}/torchvision/torchvision.cmake)
include(${DEPS_DIRECTORY}/arena-sdk/arena.cmake)
include(${DEPS_DIRECTORY}/json/json.cmake)
include(${DEPS_DIRECTORY}/eigen/eigen.cmake)
include(${DEPS_DIRECTORY}/opencv/opencv.cmake)
include(${DEPS_DIRECTORY}/httplib/httplib.cmake)
include(${DEPS_DIRECTORY}/mavsdk/mavsdk.cmake)
include(${DEPS_DIRECTORY}/matplot/matplot.cmake)
# =============================

# =============================
# obcpp executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
add_executable(${PROJECT_NAME} ${SOURCES} "src/main.cpp")
include_directories(${INCLUDE_DIRECTORY})
target_add_torch(${PROJECT_NAME})
# target_add_arena(${PROJECT_NAME}) # Tyler: currently broken, so we had to comment this out
target_add_torchvision(${PROJECT_NAME})
target_add_json(${PROJECT_NAME})
target_add_eigen(${PROJECT_NAME})
target_add_opencv(${PROJECT_NAME})
target_add_httplib(${PROJECT_NAME})
target_add_mavsdk(${PROJECT_NAME})
target_add_matplot(${PROJECT_NAME})
# =============================

# =============================
# Integration tests

# playground
add_executable(playground ${SOURCES} "tests/integration/playground.cpp")
target_include_directories(playground PRIVATE ${INCLUDE_DIRECTORY})
target_add_torch(playground)
target_add_torchvision(playground)
target_add_arena(playground)
target_add_json(playground)
target_add_eigen(playground) 
target_add_opencv(playground) 
target_add_httplib(playground)
target_add_mavsdk(playground)
target_add_matplot(playground)

# mavsdk
add_executable(mavsdk "tests/integration/mavsdk.cpp")
target_include_directories(mavsdk PRIVATE ${INCLUDE_DIRECTORY})
target_add_mavsdk(mavsdk)
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

# Add caching to clang-tidy using
# https://github.com/matus-chochlik/ctcache (doesn't work)
# stolen from https://stackoverflow.com/questions/53597508/is-it-possible-to-accelerate-clang-tidy-using-ccache-or-similar
find_program (CLANG_TIDY_CACHE_PATH NAMES "clang-tidy-cache")
if (CLANG_TIDY_CACHE_PATH)
    find_program (_CLANG_TIDY_PATH NAMES "clang-tidy" "clang-tidy-15" "clang-tidy-14" "clang-tidy-13" "clang-tidy-12")

    # Why do we use ';' here?
    # It's a cmake black magic: https://cmake.org/cmake/help/latest/prop_tgt/LANG_CLANG_TIDY.html#prop_tgt:%3CLANG%3E_CLANG_TIDY
    # The CLANG_TIDY_PATH is passed to CMAKE_CXX_CLANG_TIDY, which follows CXX_CLANG_TIDY syntax.
    set (CLANG_TIDY_PATH "${CLANG_TIDY_CACHE_PATH};${_CLANG_TIDY_PATH}" CACHE STRING "A combined command to run clang-tidy with caching wrapper")
else ()
    find_program (CLANG_TIDY_PATH NAMES "clang-tidy" "clang-tidy-15" "clang-tidy-14" "clang-tidy-13" "clang-tidy-12")
endif ()

# set(CTCACHE_DIR "/home/tuas/mycache") # testing
set (CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PATH}")

# Adding lint target if clang-tidy executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  # define lint target
  add_custom_target(lint
    COMMAND ${CMAKE_CXX_CLANG_TIDY} 
    ${SOURCES}
    ${HEADERS}
    --use-color
    --
    -std=c++${CMAKE_CXX_STANDARD}
    -I${INCLUDE_DIRECTORY}
    # NOTE:might need to add include directory if library's 
    # include directory is not in LIB_INCLUDE_CLANG_TIDY_STRING
    -I${CMAKE_BINARY_DIR}/_deps/json-src/single_include
    ${LIB_INCLUDE_CLANG_TIDY_STRING}
  )

  # run formatting using clang-tidy
  add_custom_target(format
    COMMAND ${CMAKE_CXX_CLANG_TIDY} 
    ${SOURCES}
    ${HEADERS}
    --use-color
    --fix
    --fix-errors
    --fix-notes
    --format-style=file
    -extra-arg=-std=c++${CMAKE_CXX_STANDARD}
    --
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