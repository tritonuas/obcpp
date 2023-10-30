include(FetchContent)

option(BUILD_TESTS "" OFF)
# Fetch the main OpenCV repository
FetchContent_Declare(OpenCV
  URL https://github.com/opencv/opencv/archive/refs/tags/4.8.1.tar.gz
  URL_HASH SHA256=62f650467a60a38794d681ae7e66e3e8cfba38f445e0bf87867e2f2cdc8be9d5
  DOWNLOAD_EXTRACT_TIMESTAMP true
  # CMAKE_GENERATOR ${gen}
  
  # TODO: Everything still compiles fine for me with these, but they need to be tested.
  CMAKE_ARGS
    -DBUILD_DOCS:BOOL=FALSE
    -DBUILD_EXAMPLES:BOOL=FALSE
    -DBUILD_TESTS:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=TRUE
    -DWITH_CUDA:BOOL=FALSE
    -DWITH_FFMPEG:BOOL=FALSE
    -DBUILD_PERF_TESTS:BOOL=FALSE
  OVERRIDE_FIND_PACKAGE
)

# set( OPENCV_EXTRA_MODULES_PATH ${OpenCV_SOURCE_DIR}/../opencvcontrib-src/modules)
# Fetch the OpenCV contrib repository
FetchContent_Declare(OpenCVContrib
  URL https://github.com/opencv/opencv_contrib/archive/refs/tags/4.8.1.tar.gz
  URL_HASH SHA256=0c082a0b29b3118f2a0a1856b403bb098643af7b994a0080f402a12159a99c6e
  DOWNLOAD_EXTRACT_TIMESTAMP true
  # SOURCE_DIR "${CMAKE_BINARY_DIR}/opencv_contrib-src" # Specify a separate source directory
  # PREFIX "${CMAKE_BINARY_DIR}/opencv_contrib-prefix"  # Specify a separate prefix directory
)

# Make both OpenCV and OpenCV contrib available
FetchContent_MakeAvailable(OpenCV OpenCVContrib)

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui features2d flann dnn imgcodecs videoio imgproc ml photo)
set(OpenCV_INCLUDE_DIRS ${OpenCV_SOURCE_DIR}/include )
# file(GLOB_RECURSE HEADERS "${OpenCV_SOURCE_DIR}/modules/**/*.hpp")
# Add OpenCV's contrib modules to the target
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} opencv_core opencv_highgui opencv_features2d opencv_flann opencv_imgcodecs opencv_dnn opencv_videoio opencv_imgproc opencv_ml opencv_photo )

# Specify include directories, including OpenCV's contrib
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${OpenCVContrib_SOURCE_DIR}/modules  # Include contrib modules
    ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
    ${OPENCV_MODULE_opencv_core_LOCATION}/include
    ${OPENCV_MODULE_opencv_highgui_LOCATION}/include
    ${OPENCV_MODULE_opencv_features2d_LOCATION}/include

    ${OPENCV_MODULE_opencv_flann_LOCATION}/include
    ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
    ${OPENCV_MODULE_opencv_dnn_LOCATION}/include
    ${OPENCV_MODULE_opencv_videoio_LOCATION}/include
    ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
    ${OPENCV_MODULE_opencv_ml_LOCATION}/include
    ${OPENCV_MODULE_opencv_photo_LOCATION}/include
    # ${HEADERS}
)
