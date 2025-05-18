cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Make sure that we are using the C++11 ABI, if not then there are some weird
# inconsistencies with our precompiled headers
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# set(CMAKE_POSITION_INDEPENDENT_CODE True)

SET(GCC_COMPILE_FLAGS "")
SET(GCC_LINK_FLAGS    "-lprotobuf -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_flann -lopencv_imgcodecs -lopencv_dnn -lopencv_videoio -lopencv_imgproc -lopencv_ml -lopencv_photo -lprotobuf")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

SET(CMAKE_C_FLAGS "-D_GNU_SOURCE")

# Anecdotal Observations:
#   four_jobs works with 12GB RAM
#   eight_jobs requires more than 12GB RAM
#   sixteen_jobs requires more than 16GB RAM (I don't know how much, since I only have 16GB)
# Therefore we default to 4 jobs, so it doesn't crash anyone's laptop with 12GB ram.
if (NOT DEFINED CMAKE_JOB_POOLS)
    set_property(GLOBAL PROPERTY JOB_POOLS j=4)
else()
    message("Using user defined number of pools: ${CMAKE_JOB_POOLS}")
endif()
set(CMAKE_JOB_POOL_COMPILE j)
set(CMAKE_JOB_POOL_LINK    j)

project(obcpp VERSION 1.0)

set(LOGURU_WITH_STREAMS TRUE)

# =============================
# Dependencies
set(FETCHCONTENT_QUIET FALSE)
set(DEPS_DIRECTORY ${PROJECT_SOURCE_DIR}/deps)

include(${DEPS_DIRECTORY}/onnxruntime/onnxruntime.cmake)
include(${DEPS_DIRECTORY}/torch/torch.cmake)
include(${DEPS_DIRECTORY}/torchvision/torchvision.cmake)
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

# ============================
# Global include directories

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(GEN_PROTOS_DIRECTORY ${PROJECT_SOURCE_DIR}/build/gen_protos)
include_directories(${INCLUDE_DIRECTORY} ${GEN_PROTOS_DIRECTORY})

# ============================

# =============================
# Set up imagemagick
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
# =============================
# Build speed up

# function to enable unity build for a target
function(set_unity_for_target target_name)
    set_target_properties(${target_name} PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 16 UNITY_BUILD_UNIQUE_ID "MY_UNITY_ID")
endfunction()

# =============================
# obcpp library & executable

add_subdirectory(src)

# =============================

# =============================
# Tests

add_subdirectory(${DEPS_DIRECTORY}/google-test)
add_subdirectory(tests)

# =============================

# =============================
# Pull models
add_custom_target(pull_models
  DEPENDS pull_saliency pull_matching pull_segmentation
)

# Saliency model
add_custom_target(pull_saliency
  COMMAND gdown 1S1IfXlGs_pCH49DwZmbD-tZA5YH0A1gx -O ${CMAKE_BINARY_DIR}/../models/torchscript_19.pth
  USES_TERMINAL
)

# Matching model
add_custom_target(pull_matching
  COMMAND gdown 1NeFiAfSSLXAZWlehfd0ox7p_jFF4YdrO -O ${CMAKE_BINARY_DIR}/../models/target_siamese_1.pt
  USES_TERMINAL
)


# Segmentation model
add_custom_target(pull_segmentation
  COMMAND gdown 1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf -O ${CMAKE_BINARY_DIR}/../models/fcn-model_20-epochs_06-01-2023T21-16-02.pth
  USES_TERMINAL
)
# =============================

# =============================
# Pull testing images
add_custom_target(pull_test_images
  DEPENDS pull_matching_test_images pull_saliency_test_images
)

# pull cropped images from fraternal_targets testing folder
add_custom_target(pull_matching_test_images
  COMMAND gdown 1vmP3HUS1SyqhdtJrP4QuFpGbyoyfaYSe --folder -O ${CMAKE_BINARY_DIR}/../tests/integration/images/matching
  USES_TERMINAL
)

# pull cropped images from saliency images testing folder
add_custom_target(pull_saliency_test_images
  COMMAND gdown 1JvtQUroZJHo51E37_IA2D1mfdJj2smyR --folder -O ${CMAKE_BINARY_DIR}/../tests/integration/images/saliency
  USES_TERMINAL
)

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
    # also don't check for runtime/references since Google's public style guidelines are behind https://github.com/cpplint/cpplint/issues/148
    --exclude=../include/udp_squared
    --exclude=../src/utilities/base64.cpp
    --filter=-legal,-readability/todo,-build/c++11,-runtime/references
    --linelength=100
    --recursive
    ../src
    ../include
  )
else()
    message(FATAL_ERROR "cpplint executable not found. Check the README for steps to install it on your system")
endif()
# =============================

add_custom_target(dev_protos



  COMMAND cd ${CMAKE_BINARY_DIR} &&


          cmake .. &&


          cd ${CMAKE_SOURCE_DIR}/include/udp_squared &&


          git checkout feat/udp_update &&


          cd ${CMAKE_SOURCE_DIR}/protos &&


          git checkout feat/protoupdate &&


          cd ${CMAKE_BINARY_DIR} &&


          protoc -I=../protos --cpp_out=./gen_protos/protos ../protos/obc.proto &&


          ninja obcpp


  USES_TERMINAL


)