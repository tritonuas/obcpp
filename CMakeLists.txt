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
target_add_arena(playground)
target_add_json(playground)
target_add_opencv(playground) 
target_add_httplib(playground)
target_add_mavsdk(playground)
target_add_matplot(playground)

# mavsdk
add_executable(mavsdk "tests/integration/mavsdk.cpp")
target_include_directories(mavsdk PRIVATE ${INCLUDE_DIRECTORY})
target_add_mavsdk(mavsdk)
target_add_matplot(mavsdk)

# load_torchvision_model
add_executable(load_torchvision_model tests/integration/load_torchvision_model.cpp)
target_add_torch(load_torchvision_model)
target_add_torchvision(load_torchvision_model)
# =============================

# =============================
# Unit tests
add_subdirectory(tests/unit)
add_subdirectory(${DEPS_DIRECTORY}/google-test)
# =============================

# =============================
# Linting 

# Adding lint target if cpplint executable is found
find_program(CPPLINT "cpplint")
if(CPPLINT)
  # define lint target
  add_custom_target(lint
    COMMAND cpplint
    --filter=-legal,-readability/todo
    --recursive
    ../src
    ../include
  )
else()
  message(FATAL_ERROR "cpplint executable not found. Check the README for steps to install it on your system")
endif()
# =============================