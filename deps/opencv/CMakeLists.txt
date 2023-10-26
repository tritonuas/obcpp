include(FetchContent)

# Fetch the main OpenCV repository
FetchContent_Declare(OpenCV
  URL https://github.com/opencv/opencv/archive/refs/tags/4.8.1.tar.gz
  OVERRIDE_FIND_PACKAGE
)

set( OPENCV_EXTRA_MODULES_PATH ${OpenCV_SOURCE_DIR}/../opencvcontrib-src/modules)
# Fetch the OpenCV contrib repository
FetchContent_Declare(OpenCVContrib
  URL https://github.com/opencv/opencv_contrib/archive/refs/tags/4.8.1.tar.gz
  # SOURCE_DIR "${CMAKE_BINARY_DIR}/opencv_contrib-src" # Specify a separate source directory
  # PREFIX "${CMAKE_BINARY_DIR}/opencv_contrib-prefix"  # Specify a separate prefix directory
)

# Make both OpenCV and OpenCV contrib available
FetchContent_MakeAvailable(OpenCV OpenCVContrib)

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
set(OpenCV_INCLUDE_DIRS ${OpenCV_SOURCE_DIR}/include )
file(GLOB_RECURSE HEADERS "${OpenCV_SOURCE_DIR}/modules/**/*.hpp")
# message(FATAL_ERROR ${HEADERS})
# Add OpenCV's contrib modules to the target
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} opencv_core opencv_highgui opencv_contrib)

# Specify include directories, including OpenCV's contrib
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${OpenCVContrib_SOURCE_DIR}/modules  # Include contrib modules
    ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
    ${OPENCV_MODULE_opencv_core_LOCATION}/include
    ${OPENCV_MODULE_opencv_highgui_LOCATION}/include
    # ${HEADERS}
)
