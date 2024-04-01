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
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

SET(GCC_COMPILE_FLAGS "")
SET(GCC_LINK_FLAGS    "-lprotobuf -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_flann -lopencv_imgcodecs -lopencv_dnn -lopencv_videoio -lopencv_imgproc -lopencv_ml -lopencv_photo -lprotobuf")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

SET(CMAKE_C_FLAGS "-D_GNU_SOURCE")

project(obcpp VERSION 1.0)

set(LOGURU_WITH_STREAMS TRUE)

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
include(${DEPS_DIRECTORY}/protobuf/protobuf.cmake)
include(${DEPS_DIRECTORY}/loguru/loguru.cmake)
include(${DEPS_DIRECTORY}/imagemagick/imagemagick.cmake)
# =============================

# =============================
# Set up protos

execute_process(
    COMMAND sh ${PROJECT_SOURCE_DIR}/scripts/install-protos.sh
    RESULT_VARIABLE ret
)
if(NOT (ret EQUAL "0"))
    message(FATAL_ERROR "Unable to install protos. Script exited with code ${ret}.")
endif()

# =============================

# =============================
# Set up imagemagick
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
# =============================

# =============================
# obcpp executable
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "build/gen_protos/*.pb.cc")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "build/gen_protos/*.pb.h")

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(GEN_PROTOS_DIRECTORY ${PROJECT_SOURCE_DIR}/build/gen_protos)
add_executable(${PROJECT_NAME} ${SOURCES} "src/main.cpp")
include_directories(${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
target_add_protobuf(${PROJECT_NAME})
target_add_torch(${PROJECT_NAME})
# target_add_arena(${PROJECT_NAME}) # Tyler: currently broken, so we had to comment this out
target_add_torchvision(${PROJECT_NAME})
target_add_json(${PROJECT_NAME})
target_add_opencv(${PROJECT_NAME})
target_add_httplib(${PROJECT_NAME})
target_add_mavsdk(${PROJECT_NAME})
target_add_matplot(${PROJECT_NAME})
target_add_loguru(${PROJECT_NAME})
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})
# =============================

# =============================
# Integration tests

# playground
add_executable(playground ${SOURCES} "tests/integration/playground.cpp")
target_include_directories(playground PRIVATE ${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})
target_add_torch(playground)
# target_add_arena(playground)
target_add_json(playground)
target_add_opencv(playground) 
target_add_httplib(playground)
target_add_mavsdk(playground)
target_add_matplot(playground)
target_add_protobuf(playground)
target_add_loguru(playground)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(playground PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(playground PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# load_torchvision_model
add_executable(load_torchvision_model "tests/integration/load_torchvision_model.cpp")
target_add_torch(load_torchvision_model)
target_add_torchvision(load_torchvision_model)
target_add_loguru(load_torchvision_model)

# mavlink-client
add_executable(mavlink_client ${SOURCES} "tests/integration/mavlink_client.cpp")
target_include_directories(mavlink_client PRIVATE ${INCLUDE_DIRECTORY})
target_add_protobuf(mavlink_client)
target_add_torch(mavlink_client)
target_add_torchvision(mavlink_client)
target_add_json(mavlink_client)
target_add_opencv(mavlink_client)
target_add_httplib(mavlink_client)
target_add_mavsdk(mavlink_client)
target_add_matplot(mavlink_client)
target_add_loguru(mavlink_client)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(mavlink_client PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(mavlink_client PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# cv_pipeline 
add_executable(cv_pipeline ${SOURCES} "tests/integration/cv_pipeline.cpp")
target_add_json(cv_pipeline)
target_add_matplot(cv_pipeline)
target_add_opencv(cv_pipeline)
target_add_loguru(cv_pipeline)
target_add_httplib(cv_pipeline)
target_add_protobuf(cv_pipeline)
target_add_torch(cv_pipeline)
target_add_torchvision(cv_pipeline)
target_add_mavsdk(cv_pipeline)
target_add_loguru(cv_pipeline)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_pipeline PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_pipeline PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# cv_matching
add_executable(cv_matching ${SOURCES} "tests/integration/cv_matching.cpp")
target_add_json(cv_matching)
target_add_matplot(cv_matching)
target_add_opencv(cv_matching)
target_add_torch(cv_matching)
target_add_torchvision(cv_matching)
target_add_loguru(cv_matching)
target_add_httplib(cv_matching)
target_add_mavsdk(cv_matching)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_matching PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_matching PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})


# cv_segmentation
add_executable(cv_segmentation ${SOURCES} "tests/integration/cv_segmentation.cpp")
target_add_json(cv_segmentation)
target_add_matplot(cv_segmentation)
target_add_opencv(cv_segmentation)
target_add_torch(cv_segmentation)
target_add_torchvision(cv_segmentation)
target_add_loguru(cv_segmentation)
target_add_httplib(cv_segmentation)
target_add_mavsdk(cv_segmentation)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(cv_segmentation PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(cv_segmentation PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# path_plotting 
add_executable(path_plotting ${SOURCES} tests/integration/path_plotting.cpp)
target_add_torch(path_plotting)
target_add_json(path_plotting)
target_add_httplib(path_plotting)
target_add_mavsdk(path_plotting)
target_add_matplot(path_plotting)
target_add_protobuf(path_plotting)
target_add_opencv(path_plotting) 
target_add_loguru(path_plotting)
# for some reason calling target_add_imagemagick here conflicts with, so we are including/linking without the function call
# target_add_imagemagick(path_plotting)
target_include_directories(path_plotting PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(path_plotting PRIVATE -Wl,--copy-dt-needed-entries ${ImageMagick_LIBRARIES})

# airdrop_sockets
add_executable(airdrop_sockets src/network/airdrop_sockets.c tests/integration/airdrop_sockets.c)
target_include_directories(airdrop_sockets PRIVATE ${INCLUDE_DIRECTORY})

add_executable(camera_playground tests/integration/camera_playground.cpp)
target_include_directories(camera_playground PRIVATE ${INCLUDE_DIRECTORY})
target_add_arena(camera_playground)
add_custom_target(
    run_camera_playground
    ${CMAKE_COMMAND}
    -E env LD_LIBRARY_s/target_siamese_1.pt
)
# =============================

# =============================
# Unit tests
add_subdirectory(tests/unit)
add_subdirectory(${DEPS_DIRECTORY}/google-test)
# =============================

# =============================
# Pull models
add_custom_target(pull_models 
  DEPENDS pull_saliency pull_matching pull_segmentation
)

# Saliency model
add_custom_target(pull_saliency
  COMMAND gdown https://drive.google.com/uc?id=1S1IfXlGs_pCH49DwZmbD-tZA5YH0A1gx -O ${CMAKE_BINARY_DIR}/../models/torchscript_19.pth
)

# Matching model 
add_custom_target(pull_matching
  COMMAND gdown https://drive.google.com/file/d/1NeFiAfSSLXAZWlehfd0ox7p_jFF4YdrO -O ${CMAKE_BINARY_DIR}/../models/target_siamese_1.pt


# Segmentation model
add_custom_target(pull_segmentation
  COMMAND gdown https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf -O ${CMAKE_BINARY_DIR}/../models/fcn-model_20-epochs_06-01-2023T21-16-02.pth
)
# =============================

# =============================
# Pull testing images
# 
# =============================

# =============================
# Linting 

# Adding lint target if cpplint executable is found
find_program(CPPLINT "cpplint")
if(CPPLINT)
  # define lint target
  add_custom_target(lint
    COMMAND cpplint
    # Do not require licenses, TODO assignment, Google versions of C++ libs
    --filter=-legal,-readability/todo,-build/c++11
    --linelength=100
    --recursive
    ../src
    ../include
  )
else()
  message(FATAL_ERROR "cpplint executable not found. Check the README for steps to install it on your system")
endif()
# =============================
